/**
 * @file        mainwindow.cpp
 * @brief       Main Window implementation for aiOr application.
 * @details     Implements the main user interface functionality including
 *              AI chat interactions, network communication with DeepSeek/Groq APIs,
 *              SSL error handling, markdown rendering, and API key management.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.4.2
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * par ToDo:    Proced stream response from AI
 *
 * @par Dependencies:
 * - mainwindow.h (class declaration)
 * - ui_mainwindow.h (generated UI form)
 * - aisettings.h (AI settings dialog)
 * - error_codes_deepseek.h (DeepSeek API error codes)
 * - error_codes_groq.h (Groq API error codes)
 * - api_key_reader.h (API key utility)
 *
 * @par ChangeLog:
 * 11.05.2026   v1.4.3  Arthur Markaryan - Add pretty hello to debug console"
 * 10.05.2026   v1.4.2  Arthur Markaryan - Add assistant name display instead of generic "AI:"
 * 10.05.2026   v1.4.1  Arthur Markaryan - Fix streaming response handling
 * 09.05.2026   v1.4.0  Arthur Markaryan - Integrate AI settings profiles with main window
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
#include <QTextCursor>
#include <QMetaType>

#include "utils.h"

#include "error_codes_deepseek.h"
#include "error_codes_groq.h"
#include "api_key_reader.h"

/**
 * @brief       Constructor for MainWindow.
 * @param       parent  Parent widget (default is nullptr)
 * @details     Initializes the user interface components, sets up the network manager,
 *              configures the chat history display, loads AI profiles from settings file,
 *              populates the profile combo box, and applies the first profile if available.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_currentProfileIndex(-1)
{
    ui->setupUi(this);

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, "aiOr - AI Chat Client started");

    // Configure chat history display
    ui->te_ChatHistory->setReadOnly(true);          // Read-only for display only
    ui->te_ChatHistory->setAcceptRichText(true);    // Enable rich text formatting

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
 * @brief       Destructor for MainWindow.
 * @details     Cleans up allocated UI resources. The network manager
 *              is automatically cleaned up by Qt's parent-child mechanism.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief       Handles SSL errors during network communication.
 * @param       reply   Network reply that encountered the error
 * @param       errors  List of SSL errors that occurred
 * @details     Logs all SSL errors to the chat history and ignores them
 *              to proceed with the connection. This is useful for development
 *              environments with self-signed certificates but should not be
 *              used in production without proper security review.
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
 * @brief       Handles click events on the Send button.
 * @details     Retrieves user input from the message text edit, adds it to the
 *              chat history, clears the input field, and initiates the AI request.
 *              The message is sent to the configured AI API via sendMessageToAI().
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
 * @details     Opens the AI settings dialog where users can configure
 *              API keys, model parameters, and other AI-related settings.
 *              The dialog is non-modal and auto-deletes on close.
 *              Connects to settingsChanged signal to reload profiles.
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
 * @brief       Handles AI profile selection change in combo box.
 * @param       index   Index of the selected AI profile
 * @details     Updates the AI configuration based on the selected profile
 *              and displays a confirmation message in the status bar.
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
 * @details     Reloads profiles from the settings file, updates the combo box,
 *              and applies the first profile if available. Shows a status message
 *              to inform the user that settings have been updated.
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
 * @brief       Handles completion of network requests to AI API.
 * @param       reply   Network reply containing the API response
 * @details     Processes the API response based on HTTP status codes.
 *              For streaming responses, finalizes the accumulated content.
 *              For non-streaming responses, parses the JSON response directly.
 *              Handles error codes with appropriate user messages.
 */
void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    ui->sb_Main->clearMessage();

    // Check if this is a streaming response
    if (reply == m_currentReply)
    {
        // Streaming response finished
        if (reply->error() == QNetworkReply::NoError)
        {
            // Process any remaining data in buffer
            if (!m_streamBuffer.isEmpty())
            {
                parseStreamChunk(m_streamBuffer);
            }
            // Finalize and display the accumulated response
            finalizeStreamingResponse();
        }
        else
        {
            // Handle streaming error
            ui->te_ChatHistory->append("Error: " + reply->errorString());
            m_streamingContent.clear();
        }

        // Clean up streaming resources
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        m_streamBuffer.clear();
        return;
    }

    // Non-streaming response handling
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();

        // Check for empty response
        if (response.isEmpty())
        {
            ui->te_ChatHistory->append("⚠️ Warning: Empty response from API");
            return;
        }

        parseResponse(response);
    }
    else
    {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // Get error details for debugging
        QByteArray errorData = reply->readAll();
        QString errorString = reply->errorString();

        qDebug() << "HTTP Error Code:" << httpCode;
        qDebug() << "Error String:" << errorString;
        qDebug() << "Error Data:" << QString(errorData);

        // Handle specific HTTP error codes
        switch(httpCode)
        {
        case 400:
            ui->te_ChatHistory->append("⚠️ Invalid Format: Invalid request body format.");
            ui->te_ChatHistory->append("💡 Solution: Please check your request parameters in AI Settings.");
            break;
        case 401:
            ui->te_ChatHistory->append("⚠️ Authentication Fails: Wrong API key.");
            ui->te_ChatHistory->append("💡 Solution: Please check your API key in AI Settings.");
            break;
        case 402:
            ui->te_ChatHistory->append("⚠️ Balance Error: Insufficient funds on API account.");
            ui->te_ChatHistory->append("💡 Solution: Top up your balance at your provider's website.");
            break;
        case 403:
            ui->te_ChatHistory->append("⚠️ Forbidden: Access denied to the API.");
            ui->te_ChatHistory->append("💡 Solution: Check your API key permissions and IP restrictions.");
            break;
        case 404:
            ui->te_ChatHistory->append("⚠️ Not Found: The API endpoint does not exist.");
            ui->te_ChatHistory->append("💡 Solution: Check your API URL in AI Settings.");
            break;
        case 429:
            ui->te_ChatHistory->append("⚠️ Rate limit exceeded: Too many requests.");
            ui->te_ChatHistory->append("💡 Solution: Please wait a moment before sending more messages.");
            break;
        case 500:
        case 502:
        case 503:
        case 504:
            ui->te_ChatHistory->append("⚠️ Server error: The API service is experiencing issues.");
            ui->te_ChatHistory->append("💡 Solution: Please try again later.");
            break;
        default:
            if (httpCode > 0)
            {
                ui->te_ChatHistory->append(QString("⚠️ HTTP Error %1: %2").arg(httpCode).arg(errorString));
            }
            else
            {
                ui->te_ChatHistory->append("Error: " + errorString);
            }
            break;
        }

        // Try to parse error from response body
        if (!errorData.isEmpty())
        {
            QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
            if (!errorDoc.isNull())
            {
                QJsonObject errorObj = errorDoc.object();
                if (errorObj.contains("error"))
                {
                    QJsonObject errorDetail = errorObj["error"].toObject();
                    if (errorDetail.contains("message"))
                    {
                        QString errorMessage = errorDetail["message"].toString();
                        ui->te_ChatHistory->append("Detailed error: " + errorMessage);
                    }
                }
            }
        }
    }

    reply->deleteLater();
}

/**
 * @brief       Handles streaming data received from the AI API.
 * @details     Processes SSE (Server-Sent Events) data chunks as they arrive.
 *              Appends incoming data to a buffer and processes complete messages
 *              separated by double newlines ("\n\n"). Called automatically when
 *              new data is available on the streaming reply.
 */
void MainWindow::onReadyRead()
{
    if (!m_currentReply)
    {
        return;
    }

    // Append new data to buffer
    m_streamBuffer.append(m_currentReply->readAll());

    // Process complete SSE messages (separated by double newline)
    int pos;
    while ((pos = m_streamBuffer.indexOf("\n\n")) != -1)
    {
        QByteArray chunk = m_streamBuffer.left(pos);
        m_streamBuffer.remove(0, pos + 2);

        // Skip empty chunks
        if (chunk.trimmed().isEmpty())
        {
            continue;
        }

        parseStreamChunk(chunk);
    }
}

/**
 * @brief       Parses a streaming chunk from the AI API.
 * @param       chunk Raw chunk data from the streaming response
 * @details     Extracts content from SSE format which looks like "data: {...}".
 *              Accumulates content in m_streamingContent and updates the display
 *              in real-time as each chunk arrives. Handles the "[DONE]" marker
 *              that indicates the end of the stream.
 */
void MainWindow::parseStreamChunk(const QByteArray &chunk)
{
    QString chunkStr = QString::fromUtf8(chunk);

    // SSE format: "data: {...}"
    if (chunkStr.startsWith("data: "))
    {
        QString jsonStr = chunkStr.mid(6);

        // Check for stream end marker
        if (jsonStr.trimmed() == "[DONE]")
        {
            finalizeStreamingResponse();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isNull())
        {
            QJsonObject json = doc.object();

            if (json.contains("choices"))
            {
                QJsonArray choices = json["choices"].toArray();
                if (!choices.isEmpty())
                {
                    QJsonObject choice = choices[0].toObject();

                    // Streaming responses use "delta" instead of "message"
                    if (choice.contains("delta"))
                    {
                        QJsonObject delta = choice["delta"].toObject();
                        if (delta.contains("content"))
                        {
                            QString content = delta["content"].toString();
                            if (!content.isEmpty())
                            {
                                // Accumulate content
                                m_streamingContent += content;

                                // Update display in real-time
                                QString currentText = ui->te_ChatHistory->toMarkdown();

                                // Remove last line if it starts with assistant name to avoid duplication
                                int lastNewline = currentText.lastIndexOf('\n');
                                if (lastNewline != -1)
                                {
                                    QString lastLine = currentText.mid(lastNewline + 1);
                                    QString prefix = m_currentAssistantName + ": ";
                                    if (lastLine.startsWith(prefix))
                                    {
                                        currentText = currentText.left(lastNewline);
                                    }
                                }

                                // Add updated content with assistant name
                                if (!currentText.isEmpty() && !currentText.endsWith('\n'))
                                {
                                    currentText += '\n';
                                }
                                currentText += m_currentAssistantName + ": " + m_streamingContent;
                                ui->te_ChatHistory->setMarkdown(currentText);

                                // Auto-scroll to bottom
                                QTextCursor cursor = ui->te_ChatHistory->textCursor();
                                cursor.movePosition(QTextCursor::End);
                                ui->te_ChatHistory->setTextCursor(cursor);
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief       Finalizes and displays the accumulated streaming response.
 * @details     Called when streaming is complete (when "[DONE]" marker is received).
 *              Ensures the final response is properly displayed in the chat history
 *              and cleans up the accumulated content buffer.
 */
void MainWindow::finalizeStreamingResponse()
{
    if (!m_streamingContent.isEmpty())
    {
        // Ensure the final response is properly displayed
        QString currentText = ui->te_ChatHistory->toMarkdown();

        // Remove last line if it starts with assistant name to avoid duplication
        int lastNewline = currentText.lastIndexOf('\n');
        if (lastNewline != -1)
        {
            QString lastLine = currentText.mid(lastNewline + 1);
            QString prefix = m_currentAssistantName + ": ";
            if (lastLine.startsWith(prefix))
            {
                currentText = currentText.left(lastNewline);
            }
        }

        // Add final content with assistant name
        if (!currentText.isEmpty() && !currentText.endsWith('\n'))
        {
            currentText += '\n';
        }
        currentText += m_currentAssistantName + ": " + m_streamingContent;
        ui->te_ChatHistory->setMarkdown(currentText);

        qDebug() << "Streaming response completed, length:" << m_streamingContent.length();
        m_streamingContent.clear();
    }
}

/**
 * @brief       Sends a message to the AI API.
 * @param       message User message text to send
 * @details     Constructs a JSON payload with the message, model parameters,
 *              and authentication header. Configures SSL settings and sends
 *              an asynchronous POST request. For streaming mode, sets up
 *              special handlers to process data as it arrives. For non-streaming
 *              mode, uses the simple finished handler.
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

    QUrl url(ai.url);
    QNetworkRequest request(url);

    // SSL configuration - use TLS 1.2 or later
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Request headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(ai.apiKey).toUtf8());

    // JSON payload construction
    QJsonObject json;
    json["model"] = ai.model;

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = ai.max_tokens;
    json["temperature"] = ai.temperature;
    json["stream"] = ai.stream;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    qDebug() << "Sending request to:" << ai.url;
    qDebug() << "Stream mode:" << (ai.stream ? "enabled" : "disabled");
    qDebug() << "Request body:" << QString(data);

    if (ai.stream)
    {
        // Streaming mode: handle data as it arrives
        m_streamingContent.clear();
        m_streamBuffer.clear();

        QNetworkReply *reply = networkManager->post(request, data);
        m_currentReply = reply;

        // Connect using lambdas to avoid signal/slot argument mismatch
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            if (reply == m_currentReply)
            {
                onReadyRead();
            }
        });

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply == m_currentReply)
            {
                onReplyFinished(reply);
            }
        });

        ui->sb_Main->showMessage(QString("Streaming request to %1...").arg(ai.model));
    }
    else
    {
        // Non-streaming mode: use simple finished handler
        // Disconnect any previous connections to avoid duplicate handling
        disconnect(networkManager, &QNetworkAccessManager::finished,
                   this, &MainWindow::onReplyFinished);

        connect(networkManager, &QNetworkAccessManager::finished,
                this, &MainWindow::onReplyFinished);

        networkManager->post(request, data);
        ui->sb_Main->showMessage(QString("Sending query to %1...").arg(ai.model));
    }
}

/**
 * @brief       Loads profiles from AiSettings configuration file.
 * @details     Reads the aisettings.set file (INI format) from the application
 *              directory and populates the m_profiles list with all configured
 *              AI profiles. The file format includes a list of profile names
 *              and a separate array of settings that correspond to each profile.
 *              Handles missing files gracefully and properly converts the stream
 *              value which may be stored as bool, int, or string.
 */
void MainWindow::loadProfilesFromSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/aisettings.set";
    QSettings settings(configPath, QSettings::IniFormat);

    m_profiles.clear();

    qDebug() << "Loading profiles from:" << configPath;

    // If settings file doesn't exist, use default profiles as fallback
    if (!QFile::exists(configPath))
    {
        qDebug() << "AI settings file does not exist, using default fallback profiles";

        ProfileInfo defaultProfile;
        defaultProfile.name = "DeepSeek (Default)";
        defaultProfile.model = "deepseek-chat";
        defaultProfile.url = "https://api.deepseek.com/v1/chat/completions";
        defaultProfile.apiKey = "";
        defaultProfile.max_tokens = 4000;
        defaultProfile.temperature = 0.3;
        defaultProfile.stream = false;
        m_profiles.append(defaultProfile);
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

            // Handle stream value that could be stored in different formats (Qt 6 compatible)
            QVariant streamValue = settings.value("stream");

            // Use metaType() instead of deprecated type() for Qt 6
            if (streamValue.metaType().id() == QMetaType::Bool)
            {
                profile.stream = streamValue.toBool();
            }
            else if (streamValue.metaType().id() == QMetaType::Int ||
                     streamValue.metaType().id() == QMetaType::UInt ||
                     streamValue.metaType().id() == QMetaType::LongLong ||
                     streamValue.metaType().id() == QMetaType::ULongLong)
            {
                profile.stream = (streamValue.toInt() != 0);
            }
            else if (streamValue.metaType().id() == QMetaType::QString)
            {
                QString str = streamValue.toString().toLower();
                profile.stream = (str == "true" || str == "1" || str == "yes");
            }
            else
            {
                profile.stream = false;
            }
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
                 << "stream:" << profile.stream;
    }
    settings.endArray();

    // If no profiles were loaded, report the issue
    if (m_profiles.isEmpty())
    {
        qDebug() << "No profiles found in settings file";
    }

    qDebug() << "Profiles loaded successfully, count:" << m_profiles.size();
}

/**
 * @brief       Parses the AI API response (non-streaming mode).
 * @param       response Raw JSON response from the API
 * @details     Extracts the AI-generated content from the response JSON.
 *              The expected format is: {"choices":[{"message":{"content":"..."}}]}
 *              Handles error responses and displays appropriate messages.
 *              Successfully parsed content is rendered as markdown.
 */
void MainWindow::parseResponse(const QByteArray &response)
{
    // Clean the response: remove leading/trailing whitespace, BOM characters, etc.
    QByteArray cleanResponse = response.trimmed();

    // Remove UTF-8 BOM if present (EF BB BF)
    if (cleanResponse.size() >= 3 &&
        (unsigned char)cleanResponse[0] == 0xEF &&
        (unsigned char)cleanResponse[1] == 0xBB &&
        (unsigned char)cleanResponse[2] == 0xBF)
    {
        cleanResponse = cleanResponse.mid(3);
        cleanResponse = cleanResponse.trimmed();
    }

    qDebug() << "Response received (cleaned):" << QString(cleanResponse);

    // Check if response is empty
    if (cleanResponse.isEmpty())
    {
        ui->te_ChatHistory->append("Response parsing error: Empty response from API");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(cleanResponse);
    if (!doc.isNull())
    {
        QJsonObject json = doc.object();

        // Check for successful response with choices
        if (json.contains("choices"))
        {
            QJsonArray choices = json["choices"].toArray();
            if (!choices.isEmpty())
            {
                QJsonObject choice = choices[0].toObject();

                if (choice.contains("message"))
                {
                    QJsonObject message = choice["message"].toObject();
                    QString content = message["content"].toString();
                    if (!content.isEmpty())
                    {
                        // Use assistant name as prefix
                        appendMarkdown(ui->te_ChatHistory, content, m_currentAssistantName + ": ");
                        return;
                    }
                }

                // Some APIs use "text" instead of "message"
                if (choice.contains("text"))
                {
                    QString content = choice["text"].toString();
                    if (!content.isEmpty())
                    {
                        // Use assistant name as prefix
                        appendMarkdown(ui->te_ChatHistory, content, m_currentAssistantName + ": ");
                        return;
                    }
                }
            }
        }

        // Check for error response
        if (json.contains("error"))
        {
            QJsonObject error = json["error"].toObject();
            QString errorMsg = error["message"].toString();
            ui->te_ChatHistory->append("API Error: " + errorMsg);
            qDebug() << "API Error:" << errorMsg;
        }
        else
        {
            ui->te_ChatHistory->append("Response parsing error: Unexpected response format");
            qDebug() << "Unexpected response JSON:" << QString(cleanResponse);
        }
    }
    else
    {
        // Try to extract error message from non-JSON response
        QString responseStr = QString::fromUtf8(cleanResponse);
        if (responseStr.contains("<!DOCTYPE") || responseStr.contains("<html>"))
        {
            ui->te_ChatHistory->append("API Error: Received HTML error page instead of JSON");
            ui->te_ChatHistory->append("💡 Solution: Check your API URL and network connection.");
        }
        else if (responseStr.length() > 0)
        {
            ui->te_ChatHistory->append("Response parsing error: Invalid JSON format");
            qDebug() << "Invalid JSON response:" << responseStr;
        }
        else
        {
            ui->te_ChatHistory->append("Response parsing error: Empty or invalid response");
        }
    }
}

/**
 * @brief       Suggests alternative AI services to the user.
 * @details     Called when API errors occur. Displays a local response message
 *              suggesting users check their configuration, API key, account balance,
 *              and network connection. Provides a helpful fallback when the
 *              primary AI service is unavailable.
 */
void MainWindow::suggestAlternative()
{
    ui->te_ChatHistory->append("🤖 Local response: Unable to connect to the AI service. "
                               "Please check your API key, account balance, and network connection. "
                               "You can also try switching to a different AI profile.");
}

/**
 * @brief       Appends Markdown text to a QTextEdit widget.
 * @param       textEdit    Target QTextEdit widget
 * @param       markdown    Markdown text to append
 * @param       prefix      Optional prefix for the message (default is empty)
 * @details     Retrieves the current markdown content, appends the new text
 *              with a new line separator, and sets the updated markdown back.
 *              Preserves existing formatting and content. Uses QTextEdit's
 *              built-in markdown support for proper rendering.
 */
void MainWindow::appendMarkdown(QTextEdit* textEdit, const QString& markdown, const QString& prefix)
{
    // Get current text in markdown format
    QString currentText = textEdit->toMarkdown();

    // Add new line and new text with prefix
    if (!currentText.isEmpty() && !currentText.endsWith('\n'))
    {
        currentText += '\n';
    }
    currentText += prefix + markdown;

    // Set the updated text
    textEdit->setMarkdown(currentText);

    // Auto-scroll to bottom
    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);
}

/**
 * @brief       Appends Markdown converted to HTML.
 * @param       textEdit    Target QTextEdit widget
 * @param       markdown    Markdown text to convert and append
 * @details     Converts markdown formatting to rich HTML for enhanced display.
 *              Strips outer HTML body tags to preserve only the content.
 *              Uses QTextEdit::append() which accepts HTML formatting.
 *              This method is kept for compatibility but appendMarkdown()
 *              is preferred for normal use.
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

    textEdit->append(html);
}

/**
 * @brief       Updates the combo box with loaded profile names.
 * @details     Clears the existing combo box items, repopulates with
 *              profile names from m_profiles, and restores the selection
 *              to the previously selected profile. Temporarily disconnects
 *              the currentIndexChanged signal to avoid triggering callbacks
 *              while rebuilding the list.
 */
void MainWindow::updateProfileComboBox()
{
    // Temporarily disconnect signal to avoid triggering on each change
    disconnect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &MainWindow::on_cb_AI_currentIndexChanged);

    ui->cb_AI->clear();

    // Add all profile names to combo box
    for (const ProfileInfo &profile : m_profiles)
    {
        ui->cb_AI->addItem(profile.name);
    }

    // Restore selection if valid
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
 * @details     Updates the ai structure with settings from the specified profile,
 *              updates m_currentProfileIndex, and logs the applied settings.
 *              Shows warning messages in the chat history if required settings
 *              (URL or API key) are missing, as this will prevent successful
 *              API communication.
 */
void MainWindow::applyProfileSettings(int profileIndex)
{
    if (profileIndex < 0 || profileIndex >= m_profiles.size())
    {
        qDebug() << "Invalid profile index:" << profileIndex;
        return;
    }

    const ProfileInfo &profile = m_profiles[profileIndex];

    // Apply all settings to active AI configuration
    ai.model = profile.model;
    ai.url = profile.url;
    ai.apiKey = profile.apiKey;
    ai.max_tokens = profile.max_tokens;
    ai.temperature = profile.temperature;
    ai.stream = profile.stream;
    m_currentAssistantName = profile.name;  // Store assistant name

    m_currentProfileIndex = profileIndex;

    qDebug() << "Applied profile:" << profile.name
             << "Model:" << ai.model
             << "URL:" << ai.url
             << "Max tokens:" << ai.max_tokens
             << "Temperature:" << ai.temperature
             << "Stream:" << ai.stream
             << "Assistant name:" << m_currentAssistantName;

    // Validate and warn about missing settings
    if (ai.url.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Warning: No API URL configured for this profile.");
    }
    if (ai.apiKey.isEmpty())
    {
        ui->te_ChatHistory->append("⚠️ Warning: No API key configured for this profile.");
    }

    // Show stream status in status bar
    if (ai.stream)
    {
        ui->sb_Main->showMessage(QString("Profile '%1' loaded (Streaming mode enabled)").arg(profile.name), 3000);
    }
}
