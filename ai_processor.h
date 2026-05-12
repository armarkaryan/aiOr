/**
 * @file        ai_processor.h
 * @brief       AI Processor header for aiOr application.
 * @details     Contains the AiProcessor class which handles all AI API communication
 *              including network requests, response parsing (both streaming and
 *              non-streaming), and error handling. This class is non-graphical.
 *
 * @author      Arthur Markaryan
 * @date        12.05.2026
 * @version     1.0
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par ChangeLog:
 * 12.05.2026   v1.0    Arthur Markaryan - Initial implementation (based on old MainWindow)
 */

#ifndef _AI_PROCESSOR_H_
#define _AI_PROCESSOR_H_

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QString>
#include <QList>

/**
 * @brief       AI configuration structure.
 * @details     Contains all parameters required for API communication
 *              including model selection, endpoint URL, authentication key,
 *              and generation parameters.
 */
struct AiConfig
{
    QString name = "";                          //!< Profile display name
    QString model = "";                         //!< AI model name
    QString url = "";                           //!< API endpoint URL
    QString apiKey = "";                        //!< API authentication key
    int max_tokens = 2048;                      //!< Maximum response tokens
    double temperature = 0.7;                   //!< Response randomness (0.0-1.0)
    bool stream = false;                        //!< Streaming mode flag
    QString assistantName = "AI";               //!< Assistant display name
};

/**
 * @brief       AI Processor class for handling AI API communication.
 * @details     Provides methods for sending messages to AI APIs (DeepSeek, Groq, etc.),
 *              processing responses (both streaming and non-streaming),
 *              and signal emissions for UI updates. This class is non-graphical
 *              and can be used independently of any UI framework.
 */
class AiProcessor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief       Constructor for AiProcessor.
     * @param       parent  Parent QObject (default is nullptr)
     */
    explicit AiProcessor(QObject *parent = nullptr);

    /**
     * @brief       Destructor for AiProcessor.
     */
    ~AiProcessor();

    /**
     * @brief       Sets the AI configuration.
     * @param       config  New AI configuration
     */
    void setConfig(const AiConfig &config);

    /**
     * @brief       Gets the current AI configuration.
     * @return      Current AI configuration
     */
    AiConfig getConfig() const;

    /**
     * @brief       Sends a message to the AI API.
     * @param       message User message text to send
     * @details     Constructs a JSON payload with the message, model parameters,
     *              and authentication header. Configures SSL settings and sends
     *              an asynchronous POST request. Emits appropriate signals
     *              for response handling.
     */
    void sendMessage(const QString &message);

    /**
     * @brief       Cancels the current active request.
     * @details     Aborts the current network reply and cleans up resources.
     */
    void cancelCurrentRequest();

    /**
     * @brief       Checks if a request is currently in progress.
     * @return      True if a request is active, false otherwise
     */
    bool isRequestInProgress() const;

signals:
    /**
     * @brief       Emitted when a streaming chunk is received.
     * @param       chunk   Partial content chunk received
     * @param       accumulated Full accumulated content so far
     */
    void streamChunkReceived(const QString &chunk, const QString &accumulated);

    /**
     * @brief       Emitted when streaming response is complete.
     * @param       fullResponse   Complete accumulated response content
     */
    void streamCompleted(const QString &fullResponse);

    /**
     * @brief       Emitted when a non-streaming response is received.
     * @param       response    Complete response content
     */
    void responseReceived(const QString &response);

    /**
     * @brief       Emitted when an error occurs.
     * @param       errorMessage    Human-readable error message
     * @param       errorCode       HTTP status code or network error code (0 if N/A)
     */
    void errorOccurred(const QString &errorMessage, int errorCode);

    /**
     * @brief       Emitted when a request starts.
     * @param       model   Model being used for the request
     * @param       isStreaming Whether streaming mode is enabled
     */
    void requestStarted(const QString &model, bool isStreaming);

    /**
     * @brief       Emitted when a request finishes (success or error).
     */
    void requestFinished();

    /**
     * @brief       Emitted for SSL errors that occur.
     * @param       errorString    Description of SSL errors
     */
    void sslErrorOccurred(const QString &errorString);

private slots:
    /**
     * @brief       Handles SSL errors during network communication.
     * @param       reply   Network reply that encountered the error
     * @param       errors  List of SSL errors that occurred
     */
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    /**
     * @brief       Handles completion of network requests to AI API.
     * @param       reply   Network reply containing the API response
     */
    void onReplyFinished(QNetworkReply *reply);

    /**
     * @brief       Handles streaming data received from the AI API.
     * @details     Processes SSE (Server-Sent Events) data chunks as they arrive.
     *              Called automatically when new data is available.
     */
    void onReadyRead();

private:
    QNetworkAccessManager *m_networkManager;    ///< Manages network requests to AI APIs
    QNetworkReply *m_currentReply;              ///< Current network reply for streaming operations
    AiConfig m_config;                          ///< Current AI configuration
    QByteArray m_streamBuffer;                  ///< Buffer for incomplete streaming data chunks
    QString m_streamingContent;                 ///< Accumulated streaming response content
    bool m_isStreamingRequest;                  ///< Flag indicating if current request is streaming

    /**
     * @brief       Parses the AI API response (non-streaming mode).
     * @param       response Raw JSON response from the API
     * @details     Extracts the AI-generated content from the response JSON.
     *              Emits responseReceived() on success or errorOccurred() on failure.
     */
    void parseResponse(const QByteArray &response);

    /**
     * @brief       Parses a streaming chunk from the AI API.
     * @param       chunk Raw chunk data from the streaming response
     * @details     Extracts content from SSE (Server-Sent Events) format,
     *              which looks like "data: {...}". Accumulates content and
     *              emits streamChunkReceived() as chunks arrive.
     */
    void parseStreamChunk(const QByteArray &chunk);

    /**
     * @brief       Finalizes and emits the accumulated streaming response.
     * @details     Called when streaming is complete (when "[DONE]" marker is received).
     *              Emits streamCompleted() with the full accumulated content.
     */
    void finalizeStreamingResponse();

    /**
     * @brief       Builds JSON payload for the API request.
     * @param       message User message to include in the payload
     * @return      JSON document as QByteArray
     */
    QByteArray buildRequestPayload(const QString &message) const;

    /**
     * @brief       Cleans up streaming resources.
     */
    void cleanupStreamingResources();
};

#endif /* _AI_PROCESSOR_H_ */
