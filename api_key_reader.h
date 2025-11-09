#ifndef _API_KEY_READER_H_
#define _API_KEY_READER_H_

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>

class ApiKeyReader {
public:
    static QString readApiKey(const QString& filePath) {
        // Проверяем существование файла
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            qCritical() << "❌Ошибка: Файл" << filePath << "не существует";
            return QString();
        }

        // Проверяем, является ли путь файлом
        if (!fileInfo.isFile()) {
            qCritical() << "❌Ошибка:" << filePath << "не является файлом";
            return QString();
        }

        // Проверяем размер файла
        if (fileInfo.size() == 0) {
            qCritical() << "❌Ошибка: Файл" << filePath << "пуст";
            return QString();
        }

        // Проверяем права доступа
        if (!fileInfo.isReadable()) {
            qCritical() << "❌Ошибка: Нет прав на чтение файла" << filePath;
            return QString();
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "❌Ошибка открытия файла:" << file.errorString();
            return QString();
        }

        QTextStream in(&file);
        QString apiKey = in.readLine().trimmed();
        file.close();

        if (apiKey.isEmpty()) {
            qWarning() << "⚠️Предупреждение: API ключ пуст или содержит только пробелы";
        }

        return apiKey;
    }
};

#endif /* _API_KEY_READER_H_ */
