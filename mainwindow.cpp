/**
 * @file        mainwindow.cpp
 * @brief       Main Window implementation for aiOr application.
 * @details     Implements the main user interface functionality using AiProcessor
 *              for AI API communication, markdown rendering, and profile management.
 *
 * @author      Arthur Markaryan
 * @date        12.05.2026
 * @version     1.5.2
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par ToDo:   Process stream response from AI
 *
 * @par Dependencies:
 * - mainwindow.h (class declaration)
 * - ui_mainwindow.h (generated UI form)
 * - aisettings.h (AI settings dialog)
 * - error_codes_deepseek.h (DeepSeek API error codes)
 * - error_codes_groq.h (Groq API error codes)
 * - api_key_reader.h (API key utility)
 * - utils.h (debug macros)
 *
 * @par ChangeLog:
 * 12.05.2026   v1.5.2  Arthur Markaryan - Replace qDebug with UTILS_message macro
 * 12.05.2026   v1.5.1  Arthur Markaryan - Fix bug with updateProfileComboBox() double emit
 * 12.05.2026   v1.5    Arthur Markaryan - Move AI proced to the AiProcessor class
 * 11.05.2026   v1.4.3  Arthur Markaryan - Add pretty hello to debug console
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
 * 14.11.2025   v1.3    Arthur Markaryan - Added any model selection capability
 * 09.11.2025   v1.2    Arthur Markaryan - Added API-key reader
 * 09.11.2025   v1.1    Arthur Markaryan - Added error checking
 * 08.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         MainWindow
 * @see         ApiKeyReader
 * @see         AiSettings
 * @see         AiProcessor
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aisettings.h"
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTextCursor>
#include <QComboBox>

#include "api_key_reader.h"
#include "utils.h"

/**
 * @brief       Constructor for MainWindow.
 * @param       parent  Parent widget (default is nullptr)
 * @details     Initializes the user interface, configures the chat history display,
 *              connects AiProcessor signals, loads AI profiles from settings,
 *              and populates the profile selection combo box.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_aiProcessor(new AiProcessor(this))
    , m_currentProfileIndex(-1)
{
    ui->setupUi(this);

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, "aiOr - AI Chat Client started");

    // Configure chat history display
    ui->te_ChatHistory->setReadOnly(true);
    ui->te_ChatHistory->setAcceptRichText(true);

    setWindowTitle("aiOr - AI Chat Client");

    // Connect AiProcessor signals
    connect(m_aiProcessor, &AiProcessor::streamChunkReceived,
            this, &MainWindow::onStreamChunkReceived);
    connect(m_aiProcessor, &AiProcessor::streamCompleted,
            this, &MainWindow::onStreamCompleted);
    connect(m_aiProcessor, &AiProcessor::responseReceived,
            this, &MainWindow::onResponseReceived);
    connect(m_aiProcessor, &AiProcessor::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    connect(m_aiProcessor, &AiProcessor::requestStarted,
            this, &MainWindow::onRequestStarted);
    connect(m_aiProcessor, &AiProcessor::requestFinished,
            this, &MainWindow::onRequestFinished);

    // Load profiles from settings file
    loadProfilesFromSettings();

    // Update combo box with profile names
    updateProfileComboBox();
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
 *              chat history, clears the input field, and forwards the message
 *              to AiProcessor for API communication.
 */
void MainWindow::on_pb_Send_clicked()
{
    QString message = ui->te_Message->toPlainText().trimmed();
    if (!message.isEmpty())
    {
        // Add user message to chat history
        appendToChat(message, "You: ");
        ui->te_Message->clear();

        // Send request to AI API via AiProcessor
        m_aiProcessor->sendMessage(message);
    }
}

/**
 * @brief       Handles click events on the AI Settings button.
 * @details     Creates and displays the AI Settings dialog. The dialog is set
 *              to delete itself when closed (WA_DeleteOnClose). Connects to
 *              the settingsChanged signal to reload profiles when settings are updated.
 */
void MainWindow::on_tb_AI_Settings_clicked()
{
    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, "Settings button clicked!");

    AiSettings *settingsDialog = new AiSettings(this);
    settingsDialog->setWindowTitle("AI Settings");
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(settingsDialog, &AiSettings::settingsChanged,
            this, &MainWindow::onSettingsChanged);

    settingsDialog->show();
}

/**
 * @brief       Handles AI profile selection change in combo box.
 * @param       index   Index of the selected AI profile
 * @details     Checks if the selected profile is different from the current one
 *              before applying changes to prevent unnecessary reconfiguration.
 *              Updates the status bar with the new profile name.
 */
void MainWindow::on_cb_AI_currentIndexChanged(int index)
{
    if (index >= 0 && index < m_profiles.size())
    {
        // Check if this is a different profile
        if (m_currentProfileIndex != index)
        {
            applyProfileSettings(index);
            ui->sb_Main->showMessage(QString("Switched to profile: %1").arg(m_profiles[index].name), 2000);
        }
        else
        {
            UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Same profile index, skipping reapplication: %1").arg(index).toUtf8().constData());
        }
    }
}

/**
 * @brief       Called when AI settings are changed in the settings dialog.
 * @details     Reloads all profiles from the settings file, updates the combo box,
 *              and applies the first profile if available.
 */
void MainWindow::onSettingsChanged()
{
    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, "Settings changed, reloading profiles...");

    loadProfilesFromSettings();
    updateProfileComboBox();

    if (m_profiles.size() > 0)
    {
        applyProfileSettings(0);
    }

    ui->sb_Main->showMessage("AI settings updated", 3000);
}

/**
 * @brief       Handles streaming chunk received from AiProcessor.
 * @param       chunk       Partial content chunk received (unused directly)
 * @param       accumulated Full accumulated content so far
 * @details     Updates the chat display in real-time by replacing the current
 *              streaming line with the accumulated content. Auto-scrolls to bottom.
 */
void MainWindow::onStreamChunkReceived(const QString &chunk, const QString &accumulated)
{
    Q_UNUSED(chunk)

    QString currentText = ui->te_ChatHistory->toMarkdown();
    currentText = removeStreamingLine(currentText);

    if (!currentText.isEmpty() && !currentText.endsWith('\n'))
    {
        currentText += '\n';
    }

    AiConfig config = m_aiProcessor->getConfig();
    currentText += config.assistantName + ": " + accumulated;
    ui->te_ChatHistory->setMarkdown(currentText);

    // Auto-scroll to bottom
    QTextCursor cursor = ui->te_ChatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_ChatHistory->setTextCursor(cursor);
}

/**
 * @brief       Handles streaming completion from AiProcessor.
 * @param       fullResponse   Complete accumulated response content
 * @details     Finalizes the streaming response by removing the temporary
 *              streaming line and adding the complete response.
 */
void MainWindow::onStreamCompleted(const QString &fullResponse)
{
    QString currentText = ui->te_ChatHistory->toMarkdown();
    currentText = removeStreamingLine(currentText);

    if (!currentText.isEmpty() && !currentText.endsWith('\n'))
    {
        currentText += '\n';
    }

    AiConfig config = m_aiProcessor->getConfig();
    currentText += config.assistantName + ": " + fullResponse;
    ui->te_ChatHistory->setMarkdown(currentText);

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Streaming response displayed, length: %1").arg(fullResponse.length()).toUtf8().constData());
}

/**
 * @brief       Handles non-streaming response from AiProcessor.
 * @param       response    Complete response content
 * @details     Appends the complete AI response to the chat history.
 */
void MainWindow::onResponseReceived(const QString &response)
{
    AiConfig config = m_aiProcessor->getConfig();
    appendToChat(response, config.assistantName + ": ");
}

/**
 * @brief       Handles errors from AiProcessor.
 * @param       errorMessage    Human-readable error message
 * @param       errorCode       HTTP status code or network error code
 * @details     Displays error message with helpful solution suggestions based
 *              on the error content (API URL, authentication, balance, rate limits).
 */
void MainWindow::onErrorOccurred(const QString &errorMessage, int errorCode)
{
    Q_UNUSED(errorCode)

    QString formattedMessage = "⚠️ Error: " + errorMessage;

    // Add helpful solution suggestions based on error content
    if (errorMessage.contains("API URL") || errorMessage.contains("endpoint"))
    {
        formattedMessage += "\n💡 Solution: Check your API URL in AI Settings.";
    }
    else if (errorMessage.contains("API key") || errorMessage.contains("Authentication"))
    {
        formattedMessage += "\n💡 Solution: Check your API key in AI Settings.";
    }
    else if (errorMessage.contains("Balance") || errorMessage.contains("funds"))
    {
        formattedMessage += "\n💡 Solution: Top up your balance at your provider's website.";
    }
    else if (errorMessage.contains("rate limit") || errorMessage.contains("Rate limit"))
    {
        formattedMessage += "\n💡 Solution: Please wait a moment before sending more messages.";
    }
    else if (errorMessage.contains("server error") || errorMessage.contains("Server"))
    {
        formattedMessage += "\n💡 Solution: Please try again later.";
    }

    appendToChat(formattedMessage);
}

/**
 * @brief       Handles request start from AiProcessor.
 * @param       model       Model being used for the request
 * @param       isStreaming Whether streaming mode is enabled
 * @details     Updates the status bar to indicate active request with model name and mode.
 */
void MainWindow::onRequestStarted(const QString &model, bool isStreaming)
{
    QString mode = isStreaming ? "streaming" : "standard";
    ui->sb_Main->showMessage(QString("Sending query to %1 (%2 mode)...").arg(model).arg(mode));
}

/**
 * @brief       Handles request finish from AiProcessor.
 * @details     Clears the status bar message.
 */
void MainWindow::onRequestFinished()
{
    ui->sb_Main->clearMessage();
}

/**
 * @brief       Loads profiles from AiSettings configuration file.
 * @details     Reads the 'aisettings.set' INI file from the application directory.
 *              The file contains two arrays: 'AI/PROFILES' for profile names and
 *              'AI/PROFILES_SETTINGS' for the corresponding configuration values.
 *              If the file doesn't exist, creates a default fallback profile.
 */
void MainWindow::loadProfilesFromSettings()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/aisettings.set";
    QSettings settings(configPath, QSettings::IniFormat);

    m_profiles.clear();

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Loading profiles from: %1").arg(configPath).toUtf8().constData());

    // If settings file doesn't exist, use default profiles as fallback
    if (!QFile::exists(configPath))
    {
        UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_WARNING, "AI settings file does not exist, using default fallback profiles");

        AiConfig defaultProfile;
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
            UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("  Found profile name: %1").arg(name).toUtf8().constData());
        }
    }
    settings.endArray();

    // Load settings for each AI profile
    int settingsSize = settings.beginReadArray("AI/PROFILES_SETTINGS");

    for (int i = 0; i < profileNames.size(); ++i)
    {
        AiConfig profile;
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
        UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("  Loaded profile: %1 model: %2 stream: %3").arg(profile.name).arg(profile.model).arg(profile.stream).toUtf8().constData());
    }
    settings.endArray();

    if (m_profiles.isEmpty())
    {
        UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_WARNING, "No profiles found in settings file");
    }

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Profiles loaded successfully, count: %1").arg(m_profiles.size()).toUtf8().constData());
}

/**
 * @brief       Updates the combo box with loaded profile names.
 * @details     Temporarily disconnects the signal to prevent double emissions
 *              during population. Restores the connection after updating.
 */
void MainWindow::updateProfileComboBox()
{
    // Disconnect signal temporarily
    disconnect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &MainWindow::on_cb_AI_currentIndexChanged);

    ui->cb_AI->clear();

    for (const AiConfig &profile : m_profiles)
    {
        ui->cb_AI->addItem(profile.name);
    }

    // Set the current index
    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_profiles.size())
    {
        ui->cb_AI->setCurrentIndex(m_currentProfileIndex);
    }
    else if (m_profiles.size() > 0)
    {
        ui->cb_AI->setCurrentIndex(0);
        // Do NOT update m_currentProfileIndex here - let the signal handle it
    }

    // Reconnect signal
    connect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_cb_AI_currentIndexChanged);

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Profile combo box updated with %1 items").arg(m_profiles.size()).toUtf8().constData());
}

/**
 * @brief       Applies the selected profile settings to the AI configuration.
 * @param       profileIndex Index of the profile to apply
 * @details     Configures the AiProcessor with the selected profile's settings.
 *              Validates required fields (API URL, API key) and shows warnings
 *              if they are missing. Updates the status bar with the profile name.
 */
void MainWindow::applyProfileSettings(int profileIndex)
{
    if (profileIndex < 0 || profileIndex >= m_profiles.size())
    {
        UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_ERROR, QString("Invalid profile index: %1").arg(profileIndex).toUtf8().constData());
        return;
    }

    const AiConfig &profile = m_profiles[profileIndex];

    // Apply all settings to AI processor
    m_aiProcessor->setConfig(profile);
    m_currentProfileIndex = profileIndex;

    UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_INFO, QString("Applied profile: %1 Model: %2 URL: %3 Max tokens: %4 Temperature: %5 Stream: %6").arg(profile.name).arg(profile.model).arg(profile.url).arg(profile.max_tokens).arg(profile.temperature).arg(profile.stream).toUtf8().constData());

    // Validate and warn about missing settings
    if (profile.url.isEmpty())
    {
        appendToChat("⚠️ Warning: No API URL configured for this profile.");
    }
    if (profile.apiKey.isEmpty())
    {
        appendToChat("⚠️ Warning: No API key configured for this profile.");
    }

    // Show stream status in status bar
    if (profile.stream)
    {
        ui->sb_Main->showMessage(QString("Profile '%1' loaded (Streaming mode enabled)").arg(profile.name), 3000);
    }
}

/**
 * @brief       Appends text to chat history with optional prefix.
 * @param       text    Text to append
 * @param       prefix  Optional prefix (default is empty)
 * @details     Preserves markdown formatting, adds a newline if needed,
 *              and auto-scrolls to the bottom of the chat view.
 */
void MainWindow::appendToChat(const QString &text, const QString &prefix)
{
    QString currentText = ui->te_ChatHistory->toMarkdown();

    if (!currentText.isEmpty() && !currentText.endsWith('\n'))
    {
        currentText += '\n';
    }
    currentText += prefix + text;

    ui->te_ChatHistory->setMarkdown(currentText);

    // Auto-scroll to bottom
    QTextCursor cursor = ui->te_ChatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_ChatHistory->setTextCursor(cursor);
}

/**
 * @brief       Clears the current streaming content from chat display.
 * @param       currentText    Current chat text
 * @return      Modified text with streaming line removed
 * @details     Removes the last line from the chat text if it starts with the
 *              assistant name (which indicates it's a temporary streaming line).
 *              Used during streaming updates to replace the previous chunk.
 */
QString MainWindow::removeStreamingLine(const QString &currentText) const
{
    int lastNewline = currentText.lastIndexOf('\n');
    if (lastNewline != -1)
    {
        QString lastLine = currentText.mid(lastNewline + 1);
        AiConfig config = m_aiProcessor->getConfig();
        QString prefix = config.assistantName + ": ";
        if (lastLine.startsWith(prefix))
        {
            return currentText.left(lastNewline);
        }
    }
    return currentText;
}
