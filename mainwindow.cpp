/**
 * @file        mainwindow.cpp
 * @brief       Main Window implementation for aiOr application.
 * @details     Implements the main user interface functionality including
 *              AI chat interactions, network communication with DeepSeek/Groq APIs,
 *              SSL error handling, markdown rendering, and API key management.
 *
 * @author      Arthur Markaryan
 * @date        09.05.2026
 * @version     1.3.5
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - mainwindow.h (class declaration)
 * - ui_mainwindow.h (generated UI form)
 * - error_codes_deepseek.h (DeepSeek API error codes)
 * - error_codes_groq.h (Groq API error codes)
 * - api_key_reader.h (API key utility)
 *
 * @par ChangeLog:
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
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonArray>
#include <QMessageBox>

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
{
    ui->setupUi(this);

    // Configure chat history display
    ui->te_ChatHistory->setReadOnly(true);		// Read-only for display only
    ui->te_ChatHistory->setAcceptRichText(true);	// Enable rich text formatting

    // Connect network manager signals
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);

    // Connect SSL error handler
    connect(networkManager, &QNetworkAccessManager::sslErrors,
            this, &MainWindow::onSslErrors);

    setWindowTitle("aiOr - AI Chat Client");

    // Load API key from file
    // QString filePath = "api_groq.key";
    QString filePath = "api_deepseek.key";
    QString apiKey = ApiKeyReader::readApiKey(filePath);

    if (!apiKey.isEmpty()) {
        qDebug() << "✅ API key loaded successfully.";
        ui->te_ChatHistory->append("✅ API key loaded successfully.");
        qDebug() << "Key length:" << apiKey.length() << "characters.";
        ui->te_ChatHistory->append(QString("Key length: %1 characters.").arg(apiKey.length()));

        // Use the API key in the application
        ai.apiKey = apiKey;

    } else {
        qCritical() << "❌ Failed to load API key!";
        ui->te_ChatHistory->append("❌ Failed to load API key!");
        qCritical() << "❗️Make sure the file 'api.key' exists and contains your API key!";
        ui->te_ChatHistory->append("❗️Make sure the file 'api.key' exists and contains your API key!");
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
    for (const QSslError &error : errors) {
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
 * @brief       Sends a message to the AI API.
 * @param       message User message text to send
 * @details     Constructs a JSON payload with the message, model parameters,
 *              and authentication header. Configures SSL settings and sends
 *              an asynchronous POST request to the configured AI API endpoint.
 */
void MainWindow::sendMessageToAI(const QString &message)
{
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
    json["model"] = ai.model;	// AI model selection

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = ai.max_tokens.toInt();	// Token limit
    json["temperature"] = ai.temperature.toDouble();	// Sampling temperature (0.3 for deterministic output)
    json["stream"] = false;	// Streaming disabled

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // Send request
    networkManager->post(request, data);
    ui->sb_Main->showMessage(QString("Send query to %1...").arg(ai.model));
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

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        parseResponse(response);
    } else {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        switch(httpCode)
        {
        case ERROR_CODES_DEEPSEEK_INVALID_FORMAT:		// 400 - Invalid Format
            ui->te_ChatHistory->append("⚠️ Invalid Format: Invalid request body format.");
            ui->te_ChatHistory->append("💡 Solution: Please modify your request body according to the hints in the error message.\nFor more API format details, please refer to DeepSeek API Docs.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_AUTHENTICATION_FAILS:		// 401 - Authentication Fails
            ui->te_ChatHistory->append("⚠️ Authentication Fails: Authentication fails due to the wrong API key.");
            ui->te_ChatHistory->append("💡 Solution: Please check your API key. If you don't have one, please create an API key first.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_INSUFFICIENT_BALANCE:	// 402 - Insufficient Balance
            ui->te_ChatHistory->append("⚠️ Balance Error: Insufficient funds on API account.");
            ui->te_ChatHistory->append("💡 Solution: Top up your balance at platform.deepseek.com");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_INVALID_PARAMETERS:		// 422 - Invalid Parameters
            ui->te_ChatHistory->append("⚠️ Invalid request parameters: Your request contains invalid parameters.");
            ui->te_ChatHistory->append("💡 Solution: Please modify your request parameters according to the hints in the error message.\nFor more API format details, please refer to DeepSeek API Docs.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_RATE_LIMIT_REACHED:		// 429 - Rate Limit Reached
            ui->te_ChatHistory->append("⚠️ Request rate limit exceeded: You are sending requests too quickly.");
            ui->te_ChatHistory->append("💡 Solution: Please pace your requests reasonably.\nWe also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_SERVER_ERROR:			// 500 - Server Error
            ui->te_ChatHistory->append("⚠️ Internal server error: Our server encounters an issue.");
            ui->te_ChatHistory->append("💡 Solution: Please retry your request after a brief wait and contact us if the issue persists.");
            suggestAlternative();
            break;
        case ERROR_CODES_DEEPSEEK_SERVER_OVERLOADED:		// 503 - Server Overloaded
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
    ui->te_ChatHistory->append("🤖 Local response: Hello! I cannot connect to DeepSeek API at the moment due to insufficient balance. "
                               "Please top up your account at platform.deepseek.com to continue using the neural network.");
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
    } else {
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
    if (!currentText.isEmpty() && !currentText.endsWith('\n')) {
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
    if (bodyStart != -1 && bodyEnd != -1) {
        html = html.mid(bodyStart + 6, bodyEnd - bodyStart - 6);
    }

    textEdit->append(html);	// append() works with HTML
}
