/**
 * @file        api_key_reader.h
 * @brief       API Key Reader utility for aiOr application.
 * @details     Provides a static utility class for reading API keys from text files.
 *              Includes comprehensive error checking for file existence, permissions,
 *              size, and content validation.
 *
 * @author      Arthur Markaryan
 * @date        08.05.2026
 * @version     1.0.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QFile, QTextStream, QDebug, QCoreApplication, QFileInfo)
 *
 * @par ChangeLog:
 * 08.05.2026   v1.0.1  Arthur Markaryan - Modify header of the file, translate to the English and add new comment
 * 09.11.2026   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         ApiKeyReader::readApiKey()
 */

#ifndef _API_KEY_READER_H_
#define _API_KEY_READER_H_

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>

/**
 * @brief       API Key Reader utility class.
 * @details     A static utility class that provides functionality to read API keys
 *              from plain text files with comprehensive error handling and validation.
 *              All methods are static, so no instantiation is needed.
 */
class ApiKeyReader {
public:
    /**
     * @brief       Reads an API key from a specified file.
     * @param       filePath    Path to the file containing the API key
     * @return      QString containing the API key, or empty string on failure
     * @details     Performs the following validation steps:
     *              - Checks if the file exists
     *              - Verifies the path points to a regular file
     *              - Ensures the file is not empty
     *              - Checks read permissions
     *              - Attempts to open the file for reading
     *              - Reads the first line and trims whitespace
     *
     *              If any validation fails, an appropriate error message is logged.
     *              A warning is issued if the API key is empty after trimming.
     *
     * @note        Only reads the first line of the file; any additional content is ignored.
     */
    static QString readApiKey(const QString& filePath) {
        // Check if file exists
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            qCritical() << "❌Error: File" << filePath << "does not exist";
            return QString();
        }

        // Check if path points to a regular file
        if (!fileInfo.isFile()) {
            qCritical() << "❌Error:" << filePath << "is not a regular file";
            return QString();
        }

        // Check if file is empty
        if (fileInfo.size() == 0) {
            qCritical() << "❌Error: File" << filePath << "is empty";
            return QString();
        }

        // Check read permissions
        if (!fileInfo.isReadable()) {
            qCritical() << "❌Error: No read permission for file" << filePath;
            return QString();
        }

        // Open and read the file
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "❌Error opening file:" << file.errorString();
            return QString();
        }

        QTextStream in(&file);
        QString apiKey = in.readLine().trimmed();
        file.close();

        // Warn if the API key appears empty
        if (apiKey.isEmpty()) {
            qWarning() << "⚠️Warning: API key is empty or contains only whitespace";
        }

        return apiKey;
    }
};

#endif /* _API_KEY_READER_H_ */
