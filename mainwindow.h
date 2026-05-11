/**
 * @file        mainwindow.h
 * @brief       Main Window header for aiOr application.
 * @details     Contains the MainWindow class declaration which serves as the primary
 *              user interface for the application. Manages AI chat interactions,
 *              network communication with DeepSeek/Groq APIs, and markdown rendering.
 *
 * @author      Arthur Markaryan
 * @date        10.05.2026
 * @version     1.4.2
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * par ToDo:    Proced stream response from AI
 *
 * @par ChangeLog:
 * 10.05.2026   v1.4.2  Arthur Markaryan - Add assistant name display instead of generic "AI:"
 * 10.05.2026   v1.4.1  Arthur Markaryan - Fix streaming response handling
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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief       Main Window class for aiOr application.
 * @details     Provides the primary user interface for AI chat interactions.
 *              Handles sending user messages to AI APIs (DeepSeek, Groq, Qwen),
 *              processing responses (both streaming and non-streaming),
 *              and displaying formatted markdown content.
 *              Includes SSL error handling and network request management.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief       Constructor for MainWindow.
     * @param       parent  Parent widget (default is nullptr)
     * @details     Initializes the user interface, network manager,
     *              loads AI profiles from settings, and configures
     *              the chat history display.
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
     * @details     Logs all SSL errors to the chat history and ignores them
     *              to proceed with the connection. Useful for development
     *              environments with self-signed certificates.
     */
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    /**
     * @brief       Handles click events on the Send button.
     * @details     Retrieves user input from the message text edit, adds it to the
     *              chat history, clears the input field, and initiates the AI request.
     */
    void on_pb_Send_clicked();

    /**
     * @brief       Handles completion of network requests to AI API.
     * @param       reply   Network reply containing the API response
     * @details     Processes the API response based on HTTP status codes.
     *              Handles error codes with appropriate user messages and
     *              parses successful responses. Distinguishes between
     *              streaming and non-streaming responses.
     */
    void onReplyFinished(QNetworkReply *reply);

    /**
     * @brief       Handles click events on the AI Settings button.
     * @details     Opens the AI settings dialog where users can configure
     *              API keys, model parameters, and other AI-related settings.
     *              Connects to the settingsChanged signal to reload profiles.
     */
    void on_tb_AI_Settings_clicked();

    /**
     * @brief       Handles AI profile selection change in combo box.
     * @param       index   Index of the selected AI profile
     * @details     Updates the AI configuration based on the selected profile
     *              and displays a status message in the status bar.
     */
    void on_cb_AI_currentIndexChanged(int index);

    /**
     * @brief       Called when AI settings are changed in the settings dialog.
     * @details     Reloads profiles from the settings file, updates the combo box,
     *              and applies the first profile if available.
     */
    void onSettingsChanged();

    /**
     * @brief       Handles streaming data received from the AI API.
     * @details     Processes SSE (Server-Sent Events) data chunks as they arrive.
     *              Accumulates content and updates the display in real-time.
     *              Called automatically when new data is available.
     */
    void onReadyRead();

private:
    Ui::MainWindow *ui;                         ///< Pointer to the UI components generated from .ui file
    QNetworkAccessManager *networkManager;      ///< Manages network requests to AI APIs
    QNetworkReply *m_currentReply;              ///< Current network reply for streaming operations
    QByteArray m_streamBuffer;                  ///< Buffer for incomplete streaming data chunks
    QString m_streamingContent;                 ///< Accumulated streaming response content
    QString m_currentAssistantName;             ///< Name of the currently selected assistant

    /**
     * @struct     AI
     * @brief      Configuration structure for AI API settings.
     * @details    Contains all parameters required for API communication
     *             including model selection, endpoint URL, authentication key,
     *             and generation parameters.
     *
     * @var        AI::model       AI model identifier (e.g., "deepseek-chat")
     * @var        AI::url         API endpoint URL for chat completions
     * @var        AI::apiKey      Authentication key for API access
     * @var        AI::max_tokens  Maximum number of tokens in response (1-4096)
     * @var        AI::temperature Sampling temperature (0.0 to 1.0)
     * @var        AI::stream      Enable/disable streaming responses
     */
    struct AI
    {
        QString model = "";                         //!< AI model name
        QString url = "";                           //!< API endpoint URL
        QString apiKey = "";                        //!< API authentication key
        int max_tokens = 2048;                      //!< Maximum response tokens
        double temperature = 0.7;                   //!< Response randomness (0.0-1.0)
        bool stream = false;                        //!< Streaming mode flag
    } ai;

    /**
     * @struct     ProfileInfo
     * @brief      Structure for storing complete profile information.
     * @details     Contains all settings for a single AI profile including
     *              display name and all API configuration parameters.
     *
     * @var        ProfileInfo::name        Display name of the profile
     * @var        ProfileInfo::model       AI model identifier
     * @var        ProfileInfo::url         API endpoint URL
     * @var        ProfileInfo::apiKey      API authentication key
     * @var        ProfileInfo::max_tokens  Maximum tokens for responses
     * @var        ProfileInfo::temperature Sampling temperature
     * @var        ProfileInfo::stream      Streaming mode flag
     */
    struct ProfileInfo
    {
        QString name;                               //!< Profile display name
        QString model;                              //!< AI model name
        QString url;                                //!< API endpoint URL
        QString apiKey;                             //!< API key
        int max_tokens;                             //!< Maximum tokens
        double temperature;                         //!< Temperature setting
        bool stream;                                //!< Stream flag
    };

    QList<ProfileInfo> m_profiles;                  ///< List of loaded AI profiles
    int m_currentProfileIndex;                      ///< Currently selected profile index (-1 if none)

    /**
     * @brief       Sends a message to the AI API.
     * @param       message User message text to send
     * @details     Constructs a JSON payload with the message, model parameters,
     *              and authentication header. Configures SSL settings and sends
     *              an asynchronous POST request. Handles both streaming and
     *              non-streaming modes differently.
     */
    void sendMessageToAI(const QString &message);

    /**
     * @brief       Suggests alternative AI services to the user.
     * @details     Called when API errors occur. Displays a local response message
     *              suggesting users check their API key, balance, or network connection.
     */
    void suggestAlternative();

    /**
     * @brief       Parses the AI API response (non-streaming mode).
     * @param       response Raw JSON response from the API
     * @details     Extracts the AI-generated content from the response JSON.
     *              Handles error responses and displays appropriate messages.
     *              Successfully parsed content is rendered as markdown.
     */
    void parseResponse(const QByteArray &response);

    /**
     * @brief       Parses a streaming chunk from the AI API.
     * @param       chunk Raw chunk data from the streaming response
     * @details     Extracts content from SSE (Server-Sent Events) format,
     *              which looks like "data: {...}". Accumulates content and
     *              updates the display in real-time as chunks arrive.
     */
    void parseStreamChunk(const QByteArray &chunk);

    /**
     * @brief       Finalizes and displays the accumulated streaming response.
     * @details     Called when streaming is complete (when "[DONE]" marker is received).
     *              Ensures the final response is properly displayed and cleans up.
     */
    void finalizeStreamingResponse();

    /**
     * @brief       Appends Markdown text to a QTextEdit widget.
     * @param       textEdit    Target QTextEdit widget
     * @param       markdown    Markdown text to append
     * @param       prefix      Optional prefix for the message (default is "AI: ")
     * @details     Retrieves the current markdown content, appends the new text
     *              with a new line separator, and sets the updated markdown back.
     *              Preserves existing formatting and content.
     */
    void appendMarkdown(QTextEdit* textEdit, const QString& markdown, const QString& prefix = "");

    /**
     * @brief       Appends Markdown converted to HTML.
     * @param       textEdit    Target QTextEdit widget
     * @param       markdown    Markdown text to convert and append
     * @details     Converts markdown formatting to rich HTML for enhanced display.
     *              Strips outer HTML body tags to preserve only the content.
     *              Useful for rendering formatted text, code blocks, lists, etc.
     */
    void appendAsHtml(QTextEdit* textEdit, const QString& markdown);

    /**
     * @brief       Loads profiles from AiSettings configuration file.
     * @details     Reads the aisettings.set file (INI format) and populates
     *              the m_profiles list with all configured AI profiles.
     *              Handles missing files gracefully with default profiles.
     *              Properly converts stream values from various types.
     */
    void loadProfilesFromSettings();

    /**
     * @brief       Updates the combo box with loaded profile names.
     * @details     Clears the existing combo box items, repopulates with
     *              profile names from m_profiles, and restores the selection
     *              to the previously selected profile. Temporarily disconnects
     *              signals to avoid triggering on each change.
     */
    void updateProfileComboBox();

    /**
     * @brief       Applies the selected profile settings to the AI configuration.
     * @param       profileIndex Index of the profile to apply
     * @details     Updates the ai structure with settings from the specified profile,
     *              updates m_currentProfileIndex, and shows warnings if required
     *              settings (URL or API key) are missing.
     */
    void applyProfileSettings(int profileIndex);
};

#endif /* _MAINWINDOW_H_ */
