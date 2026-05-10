/**
 * @file        mainwindow.h
 * @brief       Main Window header for aiOr application.
 * @details     Contains the MainWindow class declaration which serves as the primary
 *              user interface for the application. Manages AI chat interactions,
 *              network communication with DeepSeek/Groq APIs, and markdown rendering.
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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
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

    /**
     * @brief       Handles click events on the AI Settings button.
     * @details     Opens the AI settings window where users can configure
     *              API keys, model parameters, and other AI-related settings.
     */
    void on_tb_AI_Settings_clicked();

    /**
     * @brief       Handles AI provider selection change.
     * @param       index   Index of the selected AI provider in combo box
     * @details     Updates the AI configuration based on the selected provider.
     */
    void on_cb_AI_currentIndexChanged(int index);

    /**
     * @brief       Called when AI settings are changed in the settings dialog.
     * @details     Reloads profiles from the settings file and updates the combo box.
     */
    void onSettingsChanged();

private:
    Ui::MainWindow *ui;                         ///< Pointer to the UI components generated from .ui file
    QNetworkAccessManager *networkManager;      ///< Manages network requests to AI APIs

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
        QString model = "";                         //!< AI model
        QString url = "";                           //!< API endpoint
        QString apiKey = "";                        //!< AI API key
        int max_tokens = 2048;                      //!< Maximum tokens
        double temperature = 0.7;                   //!< Sampling temperature
        bool stream = false;                        //!< Streaming disabled
    } ai;

    /**
     * @struct     ProfileInfo
     * @brief      Structure for storing profile information.
     */
    struct ProfileInfo
    {
        QString name;                               //!< Profile name
        QString model;                              //!< AI model
        QString url;                                //!< API endpoint
        QString apiKey;                             //!< API key
        int max_tokens;                             //!< Maximum tokens
        double temperature;                         //!< Temperature
        bool stream;                                //!< Stream flag
    };

    QList<ProfileInfo> m_profiles;                  ///< List of loaded profiles
    int m_currentProfileIndex;                      ///< Currently selected profile index

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

    /**
     * @brief       Loads profiles from AiSettings configuration file.
     * @details     Reads the aisettings.set file and populates the profiles list.
     */
    void loadProfilesFromSettings();

    /**
     * @brief       Updates the combo box with loaded profile names.
     * @details     Clears and repopulates cb_AI with profile names from m_profiles.
     */
    void updateProfileComboBox();

    /**
     * @brief       Applies the selected profile settings to the AI configuration.
     * @param       profileIndex Index of the profile to apply
     * @details     Updates the ai structure with settings from the specified profile.
     */
    void applyProfileSettings(int profileIndex);
};

#endif /* _MAINWINDOW_H_ */
