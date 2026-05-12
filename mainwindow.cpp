/**
 * @file        mainwindow.cpp
 * @brief       Main Window implementation for aiOr application.
 * @details     Implements the main user interface functionality using AiProcessor
 *              for AI API communication, markdown rendering, and profile management.
 *
 * @author      Arthur Markaryan
 * @date        12.05.2026
 * @version     1.5
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
 * 12.05.2026   v1.5    Arthur Markaryan - Move AI proced to the AiProcessor class
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
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTextCursor>
#include <QComboBox>

#include "api_key_reader.h"

/**
 * @brief       Constructor for MainWindow.
 * @param       parent  Parent widget (default is nullptr)
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_aiProcessor(new AiProcessor(this))
    , m_currentProfileIndex(-1)
{
    ui->setupUi(this);

    qDebug() << "aiOr - AI Chat Client started";

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

    // Apply first profile if available
    if (m_profiles.size() > 0)
    {
        applyProfileSettings(0);
    }
}

/**
 * @brief       Destructor for MainWindow.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief       Handles click events on the Send button.
 */
void MainWindow::on_pb_Send_clicked()
{
    QString message = ui->te_Message->toPlainText().trimmed();
    if (!message.isEmpty())
    {
        // Add user message to chat history
        appendToChat(message, "You: ");
        ui->te_Message->clear();

        // Send request to AI API
        m_aiProcessor->sendMessage(message);
    }
}

/**
 * @brief       Handles click events on the AI Settings button.
 */
void MainWindow::on_tb_AI_Settings_clicked()
{
    qDebug() << "Settings button clicked!";

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
 */
void MainWindow::onSettingsChanged()
{
    qDebug() << "Settings changed, reloading profiles...";

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
 * @param       chunk       Partial content chunk received
 * @param       accumulated Full accumulated content so far
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

    qDebug() << "Streaming response displayed, length:" << fullResponse.length();
}

/**
 * @brief       Handles non-streaming response from AiProcessor.
 * @param       response    Complete response content
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
 */
void MainWindow::onRequestStarted(const QString &model, bool isStreaming)
{
    QString mode = isStreaming ? "streaming" : "standard";
    ui->sb_Main->showMessage(QString("Sending query to %1 (%2 mode)...").arg(model).arg(mode));
}

/**
 * @brief       Handles request finish from AiProcessor.
 */
void MainWindow::onRequestFinished()
{
    ui->sb_Main->clearMessage();
}

/**
 * @brief       Loads profiles from AiSettings configuration file.
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
            qDebug() << "  Found profile name:" << name;
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
        qDebug() << "  Loaded profile:" << profile.name
                 << "model:" << profile.model
                 << "stream:" << profile.stream;
    }
    settings.endArray();

    if (m_profiles.isEmpty())
    {
        qDebug() << "No profiles found in settings file";
    }

    qDebug() << "Profiles loaded successfully, count:" << m_profiles.size();
}

/**
 * @brief       Updates the combo box with loaded profile names.
 */
void MainWindow::updateProfileComboBox()
{
    disconnect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
               this, &MainWindow::on_cb_AI_currentIndexChanged);

    ui->cb_AI->clear();

    for (const AiConfig &profile : m_profiles)
    {
        ui->cb_AI->addItem(profile.name);
    }

    if (m_currentProfileIndex >= 0 && m_currentProfileIndex < m_profiles.size())
    {
        ui->cb_AI->setCurrentIndex(m_currentProfileIndex);
    }
    else if (m_profiles.size() > 0)
    {
        ui->cb_AI->setCurrentIndex(0);
        m_currentProfileIndex = 0;
    }

    connect(ui->cb_AI, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_cb_AI_currentIndexChanged);

    qDebug() << "Profile combo box updated with" << m_profiles.size() << "items";
}

/**
 * @brief       Applies the selected profile settings to the AI configuration.
 * @param       profileIndex Index of the profile to apply
 */
void MainWindow::applyProfileSettings(int profileIndex)
{
    if (profileIndex < 0 || profileIndex >= m_profiles.size())
    {
        qDebug() << "Invalid profile index:" << profileIndex;
        return;
    }

    const AiConfig &profile = m_profiles[profileIndex];

    // Apply all settings to AI processor
    m_aiProcessor->setConfig(profile);
    m_currentProfileIndex = profileIndex;

    qDebug() << "Applied profile:" << profile.name
             << "Model:" << profile.model
             << "URL:" << profile.url
             << "Max tokens:" << profile.max_tokens
             << "Temperature:" << profile.temperature
             << "Stream:" << profile.stream;

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
