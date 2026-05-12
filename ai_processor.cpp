/**
 * @file        ai_processor.cpp
 * @brief       AI Processor implementation for aiOr application.
 * @details     Implements all AI API communication logic including network
 *              requests, response parsing (both streaming and non-streaming),
 *              and error handling. This class is non-graphical.
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

#include "ai_processor.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>
#include <QDebug>

/**
 * @brief       Constructor for AiProcessor.
 * @param       parent  Parent QObject (default is nullptr)
 */
AiProcessor::AiProcessor(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_isStreamingRequest(false)
{
    // Connect SSL error handler
    connect(m_networkManager, &QNetworkAccessManager::sslErrors,
            this, &AiProcessor::onSslErrors);
}

/**
 * @brief       Destructor for AiProcessor.
 */
AiProcessor::~AiProcessor()
{
    cancelCurrentRequest();
}

/**
 * @brief       Sets the AI configuration.
 * @param       config  New AI configuration
 */
void AiProcessor::setConfig(const AiConfig &config)
{
    m_config = config;
    // Use name as assistant name by default if assistantName not set
    if (m_config.assistantName.isEmpty() || m_config.assistantName == "AI")
    {
        m_config.assistantName = m_config.name.isEmpty() ? "AI" : m_config.name;
    }
    qDebug() << "AiProcessor: Config updated - Model:" << m_config.model
             << "Stream:" << m_config.stream
             << "Assistant:" << m_config.assistantName;
}

/**
 * @brief       Gets the current AI configuration.
 * @return      Current AI configuration
 */
AiConfig AiProcessor::getConfig() const
{
    return m_config;
}

/**
 * @brief       Checks if a request is currently in progress.
 * @return      True if a request is active, false otherwise
 */
bool AiProcessor::isRequestInProgress() const
{
    return (m_currentReply != nullptr);
}

/**
 * @brief       Cancels the current active request.
 */
void AiProcessor::cancelCurrentRequest()
{
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        cleanupStreamingResources();
    }
}

/**
 * @brief       Sends a message to the AI API.
 * @param       message User message text to send
 */
void AiProcessor::sendMessage(const QString &message)
{
    // Validate configuration
    if (m_config.url.isEmpty())
    {
        emit errorOccurred("API URL is not configured. Please check AI settings.", 0);
        return;
    }

    if (m_config.apiKey.isEmpty())
    {
        emit errorOccurred("API key is not configured. Please check AI settings.", 0);
        return;
    }

    // Cancel any ongoing request
    cancelCurrentRequest();

    QUrl url(m_config.url);
    QNetworkRequest request(url);

    // SSL configuration - use TLS 1.2 or later
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Request headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_config.apiKey).toUtf8());

    // Build JSON payload
    QByteArray data = buildRequestPayload(message);

    qDebug() << "AiProcessor: Sending request to:" << m_config.url;
    qDebug() << "AiProcessor: Stream mode:" << (m_config.stream ? "enabled" : "disabled");
    qDebug() << "AiProcessor: Model:" << m_config.model;

    m_isStreamingRequest = m_config.stream;
    emit requestStarted(m_config.model, m_isStreamingRequest);

    if (m_config.stream)
    {
        // Streaming mode: handle data as it arrives
        m_streamingContent.clear();
        m_streamBuffer.clear();

        m_currentReply = m_networkManager->post(request, data);

        // Connect streaming signals - using lambda to avoid signature mismatch
        connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
            onReadyRead();
        });
        connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
            onReplyFinished(m_currentReply);
        });
    }
    else
    {
        // Non-streaming mode: use simple finished handler
        m_currentReply = m_networkManager->post(request, data);
        connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
            onReplyFinished(m_currentReply);
        });
    }
}

/**
 * @brief       Handles SSL errors during network communication.
 * @param       reply   Network reply that encountered the error
 * @param       errors  List of SSL errors that occurred
 */
void AiProcessor::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors)
    {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }

    emit sslErrorOccurred(errorString);

    // Ignore SSL errors for testing purposes (not for production!)
    reply->ignoreSslErrors();
}

/**
 * @brief       Handles completion of network requests to AI API.
 * @param       reply   Network reply containing the API response
 */
void AiProcessor::onReplyFinished(QNetworkReply *reply)
{
    if (reply != m_currentReply)
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        if (m_isStreamingRequest)
        {
            // Streaming response finished - process any remaining data
            if (!m_streamBuffer.isEmpty())
            {
                parseStreamChunk(m_streamBuffer);
            }
            finalizeStreamingResponse();
        }
        else
        {
            // Non-streaming response
            QByteArray response = reply->readAll();
            if (response.isEmpty())
            {
                emit errorOccurred("Empty response from API", 0);
            }
            else
            {
                parseResponse(response);
            }
        }
    }
    else
    {
        // Handle network error
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errorString = reply->errorString();
        QByteArray errorData = reply->readAll();

        qDebug() << "AiProcessor: HTTP Error Code:" << httpCode;
        qDebug() << "AiProcessor: Error String:" << errorString;
        qDebug() << "AiProcessor: Error Data:" << QString(errorData);

        QString userMessage;

        switch(httpCode)
        {
        case 400:
            userMessage = "Invalid Format: Invalid request body format. Please check your request parameters in AI Settings.";
            break;
        case 401:
            userMessage = "Authentication Fails: Wrong API key. Please check your API key in AI Settings.";
            break;
        case 402:
            userMessage = "Balance Error: Insufficient funds on API account. Please top up your balance at your provider's website.";
            break;
        case 403:
            userMessage = "Forbidden: Access denied to the API. Check your API key permissions and IP restrictions.";
            break;
        case 404:
            userMessage = "Not Found: The API endpoint does not exist. Check your API URL in AI Settings.";
            break;
        case 429:
            userMessage = "Rate limit exceeded: Too many requests. Please wait a moment before sending more messages.";
            break;
        case 500:
        case 502:
        case 503:
        case 504:
            userMessage = "Server error: The API service is experiencing issues. Please try again later.";
            break;
        default:
            if (httpCode > 0)
            {
                userMessage = QString("HTTP Error %1: %2").arg(httpCode).arg(errorString);
            }
            else
            {
                userMessage = errorString;
            }
            break;
        }

        // Try to extract detailed error from response body
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
                        userMessage += "\nDetailed error: " + errorMessage;
                    }
                }
            }
        }

        emit errorOccurred(userMessage, httpCode);
    }

    cleanupStreamingResources();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    emit requestFinished();
}

/**
 * @brief       Handles streaming data received from the AI API.
 */
void AiProcessor::onReadyRead()
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
 */
void AiProcessor::parseStreamChunk(const QByteArray &chunk)
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
                                m_streamingContent += content;
                                emit streamChunkReceived(content, m_streamingContent);
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief       Finalizes and emits the accumulated streaming response.
 */
void AiProcessor::finalizeStreamingResponse()
{
    if (!m_streamingContent.isEmpty())
    {
        emit streamCompleted(m_streamingContent);
        qDebug() << "AiProcessor: Streaming response completed, length:" << m_streamingContent.length();
        m_streamingContent.clear();
    }
}

/**
 * @brief       Parses the AI API response (non-streaming mode).
 * @param       response Raw JSON response from the API
 */
void AiProcessor::parseResponse(const QByteArray &response)
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

    qDebug() << "AiProcessor: Response received (cleaned):" << QString(cleanResponse);

    // Check if response is empty
    if (cleanResponse.isEmpty())
    {
        emit errorOccurred("Empty response from API", 0);
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
                        emit responseReceived(content);
                        return;
                    }
                }

                // Some APIs use "text" instead of "message"
                if (choice.contains("text"))
                {
                    QString content = choice["text"].toString();
                    if (!content.isEmpty())
                    {
                        emit responseReceived(content);
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
            emit errorOccurred(errorMsg, 0);
            qDebug() << "AiProcessor: API Error:" << errorMsg;
        }
        else
        {
            emit errorOccurred("Unexpected response format from API", 0);
            qDebug() << "AiProcessor: Unexpected response JSON:" << QString(cleanResponse);
        }
    }
    else
    {
        // Try to extract error message from non-JSON response
        QString responseStr = QString::fromUtf8(cleanResponse);
        if (responseStr.contains("<!DOCTYPE") || responseStr.contains("<html>"))
        {
            emit errorOccurred("Received HTML error page instead of JSON. Check your API URL and network connection.", 0);
        }
        else if (responseStr.length() > 0)
        {
            emit errorOccurred("Invalid JSON format received from API", 0);
            qDebug() << "AiProcessor: Invalid JSON response:" << responseStr;
        }
        else
        {
            emit errorOccurred("Empty or invalid response from API", 0);
        }
    }
}

/**
 * @brief       Builds JSON payload for the API request.
 * @param       message User message to include in the payload
 * @return      JSON document as QByteArray
 */
QByteArray AiProcessor::buildRequestPayload(const QString &message) const
{
    QJsonObject json;
    json["model"] = m_config.model;

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = m_config.max_tokens;
    json["temperature"] = m_config.temperature;
    json["stream"] = m_config.stream;

    QJsonDocument doc(json);
    return doc.toJson();
}

/**
 * @brief       Cleans up streaming resources.
 */
void AiProcessor::cleanupStreamingResources()
{
    m_streamBuffer.clear();
    m_streamingContent.clear();
    m_isStreamingRequest = false;
}
