/**
 * @file        mainwindow.h
 * @brief       Main Window header for aiOr application.
 * @details     Contains the MainWindow class declaration which serves as the primary
 *              user interface for the application. Manages AI chat interactions,
 *              network communication with DeepSeek/Groq APIs, and markdown rendering.
 *
 * @author      Arthur Markaryan
 * @date        09.05.2026
 * @version     1.3.5
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QMainWindow, QTextEdit, QNetworkAccessManager, QNetworkReply)
 * - Qt5/6 Network (QSslError)
 * - Qt5/6 Json (QJsonDocument, QJsonObject)
 *
 * @par ChangeLog:
 * 09.05.2026   v1.3.5  Arthur Markaryan - Modify header of the file
 * 06.05.2026   v1.3.4  Arthur Markaryan - Add deepseek API-key by default
 * 09.01.2026   v1.3.3  Arthur Markaryan - Replace includes from .h to .cpp file
 * 04.01.2026   v1.3.2  Arthur Markaryan - Add Groq error code header file
 * 04.01.2026   v1.3.1  Arthur Markaryan - Add Groq test
 * 14.11.2025   v1.3    Arthur Markaryan - Added any model selection capibility
 * 09.11.2025   v1.2    Arthur Markaryan - Added API-key reader
 * 09.11.2025   v1.1    Arthur Markaryan - Added error checking
 * 08.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         MainWindow::MainWindow()
 * @see         MainWindow::~MainWindow()
 * @see         MainWindow::sendMessageToAI()
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief       Main Window class for aiOr application.
 * @details     Provides the primary user interface for AI chat interactions.
 *              Handles sending user messages to AI APIs (DeepSeek or Groq),
 *              processing responses, and displaying formatted markdown content.
 *              Includes SSL error handling and network request management.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief       Constructor for MainWindow.
     * @param       parent  Parent widget (default is nullptr)
     * @details     Initializes the user interface, network manager, and AI configuration.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief       Destructor for MainWindow.
     * @details     Cleans up allocated UI and network resources.
     */
    ~MainWindow();

private slots:
    /**
     * @brief       Handles SSL errors during network communication.
     * @param       reply   Network reply that encountered the error
     * @param       errors  List of SSL errors that occurred
     * @details     Logs SSL errors and ignores them to proceed with the connection.
     *              Useful for development environments with self-signed certificates.
     */
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    /**
     * @brief       Handles click events on the Send button.
     * @details     Retrieves user input from the UI, clears the input field,
     *              and initiates the AI request by calling sendMessageToAI().
     */
    void on_pb_Send_clicked();

    /**
     * @brief       Handles completion of network requests to AI API.
     * @param       reply   Network reply containing the API response
     * @details     Processes the API response, parses error codes,
     *              and extracts the AI-generated content for display.
     */
    void onReplyFinished(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;				///< Pointer to the UI components generated from .ui file
    QNetworkAccessManager *networkManager;	///< Manages network requests to AI APIs

    /**
     * @struct     AI
     * @brief      Configuration structure for AI API settings.
     * @details    Contains all parameters required for API communication
     *             including model selection, endpoint URL, authentication key,
     *             and generation parameters (max_tokens, temperature, stream).
     *
     * @var        AI::model       AI model identifier (e.g., "deepseek-chat")
     * @var        AI::url         API endpoint URL for chat completions
     * @var        AI::apiKey      Authentication key for API access
     * @var        AI::max_tokens  Maximum number of tokens in response
     * @var        AI::temperature Sampling temperature (0.0 to 1.0)
     * @var        AI::stream      Enable/disable streaming responses
     */
    struct AI
    {
        // QString model = "deepseek-coder";					//!< Paid model
        QString model = "deepseek-chat";					//!< Free model
        QString url = "https://api.deepseek.com/v1/chat/completions";	//!< DeepSeek API endpoint
        QString apiKey = "your_deepseek_api_key_here";			//!< AI API key
        QString max_tokens = "4000";						//!< Correct (free up to 4096)
        QString temperature = "0.3";						//!< Sampling temperature
        QString stream = "false";						//!< Streaming disabled
    } ai;

    /*
    // Alternative configuration for Groq API (commented out)
    struct AI
    {
        QString model = "llama-3.3-70b-versatile";				//!< Groq model
        QString url = "https://api.groq.com/openai/v1/chat/completions";	//!< Groq API endpoint
        QString apiKey = "your_groq_api_key_here";				//!< AI API key
        QString max_tokens = "4000";						//!< Token limit
        QString temperature = "0.3";						//!< Sampling temperature
        QString stream = "false";						//!< Streaming disabled
    } ai;
*/

    /**
     * @brief       Sends a message to the AI API.
     * @param       message User message text to send
     * @details     Constructs a JSON payload with the message, model parameters,
     *              and authentication header. Sends an asynchronous POST request
     *              to the configured AI API endpoint.
     */
    void sendMessageToAI(const QString &message);

    /**
     * @brief       Suggests alternative AI services.
     * @details     Called when API errors occur. Displays a message suggesting
     *              users switch to alternative LLM providers (e.g., OpenAI)
     *              when rate limits or availability issues are encountered.
     */
    void suggestAlternative();

    /**
     * @brief       Parses the AI API response.
     * @param       response Raw JSON response from the API
     * @details     Extracts the AI-generated content from the response JSON.
     *              Handles error responses and displays appropriate messages.
     */
    void parseResponse(const QByteArray &response);

    /**
     * @brief       Appends Markdown text to a QTextEdit widget.
     * @param       textEdit    Target QTextEdit widget
     * @param       markdown    Markdown text to append
     * @details     Appends the markdown content at the end of the text edit
     *              with a new line separator. Preserves existing content.
     */
    void appendMarkdown(QTextEdit* textEdit, const QString& markdown);

    /**
     * @brief       Appends Markdown converted to HTML.
     * @param       textEdit    Target QTextEdit widget
     * @param       markdown    Markdown text to convert and append
     * @details     Converts markdown formatting to rich HTML for enhanced display.
     *              Useful for rendering formatted text, code blocks, lists, etc.
     */
    void appendAsHtml(QTextEdit* textEdit, const QString& markdown);
};

#endif /* _MAINWINDOW_H_ */
