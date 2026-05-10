/**
 * @file        mainwindow.cpp
 * @brief       Main Window implementation for aiOr application.
 * @details     Implements the main user interface functionality including
 *              AI chat interactions, network communication with DeepSeek/Groq APIs,
 *              SSL error handling, markdown rendering, and API key management.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.4.0
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par ChangeLog:
 * 10.05.2026   v1.4.0  Arthur Markaryan - Integrate AI settings profiles with main window
 * 09.05.2026   v1.3.6  Arthur Markaryan - Add AI settings window integration
 * 09.05.2026   v1.3.5  Arthur Markaryan - Modify header of the file
 * 06.05.2026   v1.3.4  Arthur Markaryan - Add deepseek API-key by default
 * 09.01.2026   v1.3.3  Arthur Markaryan - Replace includes from .h to .cpp file
 * 09.01.2026   v1.3.2.2Arthur Markaryan - Change main status bar name
 * 09.01.2026   v1.3.2.1Arthur Markaryan - Add Chat List widget class & ui
 * 04.01.2026   v1.3.2  Arthur Markaryan - Add Groq error code header file
 * 04.01.2026   v1.3.1  Arthur Markaryan - Add Groq test
 * 14.11.2025   v1.3    Arthur Markaryan - Added any model selection capibility
 * 09.11.2025   v1.2    Arthur Markaryan - Added API-key reader
 * 09.11.2025   v1.1    Arthur Markaryan - Added error checking
 * 08.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         MainWindow
 * @see         ApiKeyReader
 * @see         AiSettings
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aisettings.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonArray>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>

#include "error_codes_deepseek.h"
#include "error_codes_groq.h"
#include "api_key_reader.h"

/**
 * @brief       Constructor for MainWindow.
 * @param       parent  Parent widget (default is nullptr)
 * @details     Initializes the user interface components, sets up the network manager,
 *              configures the chat history display, loads API key from file,
 *              and establishes signal-slot connections for network operations.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
    , m_currentProfileIndex(-1)
{
    ui->setupUi(this);

    // Configure chat history display
    ui->te_ChatHistory->setReadOnly(true);          // Read-only for display only
    ui->te_ChatHistory->setAcceptRichText(true);    // Enable rich text formatting

    // Connect network manager signals
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);

    // Connect SSL error handler
    connect(networkManager, &QNetworkAccessManager::sslErrors,
            this, &MainWindow::onSslErrors);

    setWindowTitle("aiOr - AI Chat Client");

    // Load profiles from AiSettings file
    loadProfilesFromSettings();

    // Update combo box with profile names
    updateProfileComboBox();

    // Apply first profile if available
    if (m_profiles.size() > 0)
    {
        applyProfileSettings(0);
    }
}

/**
 * @brief       Handles SSL errors during network communication.
 * @param       reply   Network reply that encountered the error
 * @param       errors  List of SSL errors that occurred
 * @details     Logs all SSL errors to the chat history and ignores them
 *              to proceed with the connection. This is useful for development
 *              environments but should not be used in production.
 */
void MainWindow::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors)
    {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }

    ui->te_ChatHistory->append("SSL Errors: " + errorString);

    // Ignore SSL errors for testing purposes (not for production!)
    reply->ignoreSslErrors();
}

/**
 * @brief       Destructor for MainWindow.
 * @details     Cleans up allocated UI resources.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief       Handles click events on the Send button.
 * @details     Retrieves user input from the message text edit, adds it to the
 *              chat history, clears the input field, and initiates the AI request.
 */
void MainWindow::on_pb_Send_clicked()
{
    QString message = ui->te_Message->toPlainText().trimmed();
    if (!message.isEmpty())
    {
        // Add user message to chat history
        ui->te_ChatHistory->append("You: " + message);
        ui->te_Message->clear();

        // Send request to AI API
        sendMessageToAI(message);
    }
}

/**
 * @brief       Handles click events on the AI Settings button.
 * @details     Opens the AI settings window where users can configure
 *              API keys, model parameters, and other AI-related settings.
 */
void MainWindow::on_tb_AI_Settings_clicked()
{
    qDebug() << "Settings button clicked!";

    // Create settings dialog (non-modal to allow main window interaction)
    AiSettings *settingsDialog = new AiSettings(this);

    // Configure the settings window
    settingsDialog->setWindowTitle("AI Settings");
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose); // Auto-delete on close

    // Connect signals to receive settings changes
    connect(settingsDialog, &AiSettings::settingsChanged,
            this, &MainWindow::onSettingsChanged);

    settingsDialog->show();
}

/**
 * @brief       Handles AI provider selection change.
 * @param       index   Index of the selected AI provider in combo box
 * @details     Updates the AI configuration based on the selected provider.
 */
void MainWindow::on_cb_AI_currentIndexChanged(int index)
{
    if (index >= 0 && index < m_profiles.size())
    {
        applyProfileSettings(index);
        ui->sb_Main->showMessage(QString("Switched to profile: %1").arg(m_profiles[index].name), 2000);
    }
}

/**
 * @brief       Called when AI settings are changed in the settings dialog.
 * @details     Reloads profiles from the settings file and updates the combo box.
 */
void MainWindow::onSettingsChanged()
{
    qDebug() << "Settings changed, reloading profiles...";

    // Reload profiles from file
    loadProfilesFromSettings();

    // Update combo box
    updateProfileComboBox();

    // Apply first profile if available
    if (m_profiles.size() > 0)
    {
        applyProfileSettings(0);
    }

    ui->sb_Main->showMessage("AI settings updated", 3000);
}

/**
 * @brief       Sends a message to the AI API.
 * @param       message User message text to send
 * @details     Constructs a JSON payload with the message, model parameters,
 *              and authentication header. Configures SSL settings and sends
 *              an asynchronous POST request to the configured AI API endpoint.
 */
void MainWindow::sendMessageToAI(const QString &message)
{
    // Validate that we have required settings
    if (ai.url.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Error: API URL is not configured. Please check AI settings.");
        return;
    }

    if (ai.apiKey.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Error: API key is not configured. Please check AI settings.");
        return;
    }

    if (ai.model.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Warning: Model name is empty. Using default.");
    }

    QUrl url(ai.url);

    QNetworkRequest request(url);

    // SSL configuration
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Request headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(ai.apiKey).toUtf8());

    // JSON payload
    QJsonObject json;
    json["model"] = ai.model;   // AI model selection

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = ai.max_tokens; // Token limit
    json["temperature"] = ai.temperature;   // Sampling temperature
    json["stream"] = ai.stream; // Streaming setting

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // Send request
    networkManager->post(request, data);
    ui->sb_Main->showMessage(QString("Sending query to %1...").arg(ai.model));
}

/**
 * @brief       Handles completion of network requests to AI API.
 * @param       reply   Network reply containing the API response
 * @details     Processes the API response based on HTTP status codes.
 *              Handles error codes with appropriate user messages and
 *              alternative suggestions. Parses successful responses
 *              and displays AI-generated content.
 */
void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    ui->sb_Main->clearMessage();

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        parseResponse(response);
    }
    else
    {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        switch(httpCode)
        {
        case ERROR_CODES_DEEPSEEK_INVALID_FORMAT:       // 400 - Invalid Format
            ui->te_ChatHistory->append("⚠️ Invalid Format: Invalid request body format.");
            ui->te_ChatHistory->append("💡 Solution: Please modify your request body according to the hints in the error message.\nFor more API format details, please refer to API Docs.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_AUTHENTICATION_FAILS:     // 401 - Authentication Fails
            ui->te_ChatHistory->append("⚠️ Authentication Fails: Authentication fails due to the wrong API key.");
            ui->te_ChatHistory->append("💡 Solution: Please check your API key in AI Settings.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_INSUFFICIENT_BALANCE: // 402 - Insufficient Balance
            ui->te_ChatHistory->append("⚠️ Balance Error: Insufficient funds on API account.");
            ui->te_ChatHistory->append("💡 Solution: Top up your balance at your provider's website.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_INVALID_PARAMETERS:       // 422 - Invalid Parameters
            ui->te_ChatHistory->append("⚠️ Invalid request parameters: Your request contains invalid parameters.");
            ui->te_ChatHistory->append("💡 Solution: Please modify your request parameters according to the hints in the error message.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_RATE_LIMIT_REACHED:       // 429 - Rate Limit Reached
            ui->te_ChatHistory->append("⚠️ Request rate limit exceeded: You are sending requests too quickly.");
            ui->te_ChatHistory->append("💡 Solution: Please pace your requests reasonably.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_SERVER_ERROR:         // 500 - Server Error
            ui->te_ChatHistory->append("⚠️ Internal server error: Our server encounters an issue.");
            ui->te_ChatHistory->append("💡 Solution: Please retry your request after a brief wait and contact us if the issue persists.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_SERVER_OVERLOADED:        // 503 - Server Overloaded
            ui->te_ChatHistory->append("⚠️ Server overloaded due to high traffic: The server is overloaded due to high traffic.");
            ui->te_ChatHistory->append("💡 Solution: Please retry your request after a brief wait.");
            suggestAlternative();
            break;
        default:
            ui->te_ChatHistory->append("Error: " + reply->errorString());
            break;
        }
    }
    reply->deleteLater();
}

/**
 * @brief       Suggests alternative AI services to the user.
 * @details     Called when API errors occur. Displays a local response message
 *              suggesting users check their balance or consider alternative LLM providers.
 */
void MainWindow::suggestAlternative()
{
    QString message = ui->te_Message->toPlainText();
    ui->te_ChatHistory->append("🤖 Local response: Hello! I cannot connect to AI API at the moment. "
                               "Please check your API key, account balance, and network connection.");
}

/**
 * @brief       Parses the AI API response.
 * @param       response Raw JSON response from the API
 * @details     Extracts the AI-generated content from the response JSON.
 *              Handles error responses and displays appropriate messages.
 *              Successfully parsed content is rendered as markdown.
 */
void MainWindow::parseResponse(const QByteArray &response)
{
    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isNull())
    {
        QJsonObject json = doc.object();

        if (json.contains("choices"))
        {
            QJsonArray choices = json["choices"].toArray();
            if (!choices.isEmpty())
            {
                QJsonObject choice = choices[0].toObject();
                QJsonObject message = choice["message"].toObject();
                QString content = message["content"].toString();

                appendMarkdown(ui->te_ChatHistory, "AI: " + content);
                return;
            }
        }

        if (json.contains("error"))
        {
            QJsonObject error = json["error"].toObject();
            QString errorMsg = error["message"].toString();
            ui->te_ChatHistory->append("API Error: " + errorMsg);
        }
    }
    else
    {
        ui->te_ChatHistory->append("Response parsing error");
    }
}

/**
 * @brief       Appends Markdown text to a QTextEdit widget.
 * @param       textEdit    Target QTextEdit widget
 * @param       markdown    Markdown text to append
 * @details     Retrieves the current markdown content, appends the new text
 *              with a new line separator, and sets the updated markdown back.
 *              Preserves existing formatting and content.
 */
void MainWindow::appendMarkdown(QTextEdit* textEdit, const QString& markdown)
{
    // Get current text in markdown format
    QString currentText = textEdit->toMarkdown();

    // Add new line and new text
    if (!currentText.isEmpty() && !currentText.endsWith('\n'))
    {
        currentText += '\n';
    }
    currentText += markdown;

    // Set the updated text
    textEdit->setMarkdown(currentText);
}

/**
 * @brief       Appends Markdown converted to HTML.
 * @param       textEdit    Target QTextEdit widget
 * @param       markdown    Markdown text to convert and append
 * @details     Converts markdown formatting to rich HTML for enhanced display.
 *              Strips outer HTML body tags to preserve only the content.
 *              Uses QTextEdit::append() which accepts HTML formatting.
 */
void MainWindow::appendAsHtml(QTextEdit* textEdit, const QString& markdown)
{
    QTextEdit tempEdit;
    tempEdit.setHtml(markdown);
    QString html = tempEdit.toHtml();

    // Remove outer HTML tags, keep only body content
    int bodyStart = html.indexOf("<body>");
    int bodyEnd = html.indexOf("</body>");
    if (bodyStart != -1 && bodyEnd != -1)
    {
        html = html.mid(bodyStart + 6, bodyEnd - bodyStart - 6);
    }

    textEdit->append(html); // append() works with HTML
}

/**
 * @brief       Loads profiles from AiSettings configuration file.
 * @details     Reads the aisettings.set file and populates the profiles list.
 */
void MainWindow::loadProfilesFromSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/aisettings.set";
    QSettings settings(configPath, QSettings::IniFormat);

    m_profiles.clear();

    qDebug() << "Loading profiles from:" << configPath;

    if (!QFile::exists(configPath))
    {
        qDebug() << "AI settings file does not exist, using hardcoded defaults";

        // Add default profiles as fallback
        ProfileInfo defaultProfile;
        defaultProfile.name = "DeepSeek (Default)";
        defaultProfile.model = "deepseek-chat";
        defaultProfile.url = "https://api.deepseek.com/v1/chat/completions";
        defaultProfile.apiKey = "";
        defaultProfile.max_tokens = 4000;
        defaultProfile.temperature = 0.3;
        defaultProfile.stream = false;
        m_profiles.append(defaultProfile);

        ProfileInfo groqProfile;
        groqProfile.name = "Groq (Alternative)";
        groqProfile.model = "llama-3.3-70b-versatile";
        groqProfile.url = "https://api.groq.com/openai/v1/chat/completions";
        groqProfile.apiKey = "";
        groqProfile.max_tokens = 4000;
        groqProfile.temperature = 0.3;
        groqProfile.stream = false;
        m_profiles.append(groqProfile);

        return;
    }

    // Load AI list items (profile names)
    int listSize = settings.beginReadArray("AI/PROFILES");
    QStringList profileNames;

    for (int i = 0; i < listSize; ++i)
    {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        if (!name.isEmpty())
        {
            profileNames.append(name);
            qDebug() << "  Found profile name:" << name;
        }
    }
    settings.endArray();

    // Load settings for each AI profile
    int settingsSize = settings.beginReadArray("AI/PROFILES_SETTINGS");

    for (int i = 0; i < profileNames.size(); ++i)
    {
        ProfileInfo profile;
        profile.name = profileNames[i];

        if (i < settingsSize)
        {
            settings.setArrayIndex(i);
            profile.model = settings.value("model").toString();
            profile.url = settings.value("url").toString();
            profile.apiKey = settings.value("api_key").toString();
            profile.max_tokens = settings.value("max_tokens", 2048).toInt();
            profile.temperature = settings.value("temperature", 0.7).toDouble();
            profile.stream = settings.value("stream", false).toBool();
        }
        else
        {
            // Default settings for profiles without saved settings
            profile.model = "";
            profile.url = "";
            profile.apiKey = "";
            profile.max_tokens = 2048;
            profile.temperature = 0.7;
            profile.stream = false;
        }

        m_profiles.append(profile);
        qDebug() << "  Loaded profile:" << profile.name
                 << "model:" << profile.model
                 << "url:" << profile.url;
    }
    settings.endArray();

    // If no profiles were loaded, add a default one
    if (m_profiles.isEmpty())
    {
        ProfileInfo defaultProfile;
        defaultProfile.name = "Default AI";
        defaultProfile.model = "";
        defaultProfile.url = "";
        defaultProfile.apiKey = "";
        defaultProfile.max_tokens = 2048;
        defaultProfile.temperature = 0.7;
        defaultProfile.stream = false;
        m_profiles.append(defaultProfile);
        qDebug() << "No profiles found, created default profile";
    }

    // Load current profile selection
    int currentProfile = settings.value("AI/CURRENT_PROFILE", 0).toInt();
    if (currentProfile >= 0 && currentProfile < m_profiles.size())
    {
        m_currentProfileIndex = currentProfile;
        qDebug() << "Saved current profile index:" << currentProfile;
    }
    else
    {
        m_currentProfileIndex = 0;
    }

    qDebug() << "Profiles loaded successfully, count:" << m_profiles.size();
}

/**
 * @brief       Updates the combo box with loaded profile names.
 * @details     Clears and repopulates cb_AI with profile names from m_profiles.
 */
void MainWindow::updateProfileComboBox()
{
    // Temporarily disconnect signal to avoid triggering on each change
    disconnect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &MainWindow::on_cb_AI_currentIndexChanged);

    ui->cb_AI->clear();

    for (const ProfileInfo &profile : m_profiles)
    {
        ui->cb_AI->addItem(profile.name);
    }

    // Restore selection
    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_profiles.size())
    {
        ui->cb_AI->setCurrentIndex(m_currentProfileIndex);
    }
    else if (m_profiles.size() > 0)
    {
        ui->cb_AI->setCurrentIndex(0);
        m_currentProfileIndex = 0;
    }

    // Reconnect signal
    connect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_cb_AI_currentIndexChanged);

    qDebug() << "Profile combo box updated with" << m_profiles.size() << "items";
}

/**
 * @brief       Applies the selected profile settings to the AI configuration.
 * @param       profileIndex Index of the profile to apply
 * @details     Updates the ai structure with settings from the specified profile.
 */
void MainWindow::applyProfileSettings(int profileIndex)
{
    if (profileIndex < 0 || profileIndex >= m_profiles.size())
    {
        qDebug() << "Invalid profile index:" << profileIndex;
        return;
    }

    const ProfileInfo &profile = m_profiles[profileIndex];

    ai.model = profile.model;
    ai.url = profile.url;
    ai.apiKey = profile.apiKey;
    ai.max_tokens = profile.max_tokens;
    ai.temperature = profile.temperature;
    ai.stream = profile.stream;

    m_currentProfileIndex = profileIndex;

    qDebug() << "Applied profile:" << profile.name
             << "Model:" << ai.model
             << "URL:" << ai.url
             << "Max tokens:" << ai.max_tokens
             << "Temperature:" << ai.temperature
             << "Stream:" << ai.stream;

    // Validate and warn about missing settings
    if (ai.url.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Warning: No API URL configured for this profile.");
    }
    if (ai.apiKey.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Warning: No API key configured for this profile.");
    }
}
