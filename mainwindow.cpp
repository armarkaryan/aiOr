#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonArray>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);

    // Обработка SSL ошибок
    connect(networkManager, &QNetworkAccessManager::sslErrors,
            this, &MainWindow::onSslErrors);

    setWindowTitle("aiOr - AI Chat Client");

    //
    QString filePath = "api.key";
    QString apiKey = ApiKeyReader::readApiKey(filePath);

    if (!apiKey.isEmpty()) {
        qDebug() << "✅ API ключ успешно загружен.";
        ui->te_ChatHistory->append("✅ API ключ успешно загружен.");
        qDebug() << "Длина ключа:" << apiKey.length() << "символов.";
        ui->te_ChatHistory->append(QString("Длина ключа: %1 символов.").arg(apiKey.length()));

        // Используйте apiKey в вашем коде
        // Например: your_api_function(apiKey);
        ai.apiKey = apiKey;

    } else {
        qCritical() << "❌ Не удалось загрузить API ключ!";
        ui->te_ChatHistory->append("❌ Не удалось загрузить API ключ!");
        qCritical() << "❗️Убедитесь, что файл 'api.key' существует и содержит ваш API ключ!";
        ui->te_ChatHistory->append("❗️Убедитесь, что файл 'api.key' существует и содержит ваш API ключ!");
    }
}

void MainWindow::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += ", ";
        errorString += error.errorString();
    }

    ui->te_ChatHistory->append("SSL Errors: " + errorString);

    // Можно игнорировать ошибки для тестирования (не для production!)
    reply->ignoreSslErrors();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pb_Send_clicked()
{
    QString message = ui->te_Message->toPlainText().trimmed();
    if (!message.isEmpty())
    {
        // Добавляем сообщение в историю
        ui->te_ChatHistory->append("Вы: " + message);
        ui->te_Message->clear();

        // Отправляем запрос к DeepSeek API
        sendMessageToAI(message);
    }
}

void MainWindow::sendMessageToAI(const QString &message)
{
    //QUrl url("https://api.deepseek.com/v1/chat/completions");
    QUrl url(ai.url);

    QNetworkRequest request(url);

    // SSL конфигурация
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Заголовки
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //request.setRawHeader("Authorization", QString("Bearer %1").arg(deepSeekApiKey).toUtf8());
    request.setRawHeader("Authorization", QString("Bearer %1").arg(ai.apiKey).toUtf8());

    // JSON данные
    QJsonObject json;
    //json["model"] = "deepseek-coder"; // Специализированная модель для программирования
    json["model"] = ai.model; // Специализированная модель для программирования

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    //json["max_tokens"] = 4000; // Увеличил для кода
    json["max_tokens"] = ai.max_tokens.toInt(); // Увеличил для кода
    //json["temperature"] = 0.3; // Понизил для более детерминированного кода
    json["temperature"] = ai.temperature.toDouble(); // Понизил для более детерминированного кода 0.3
    json["stream"] = false;
    //json["stream"] = ai.stream;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // Отправка
    networkManager->post(request, data);
    ui->statusBar->showMessage(QString("Send quarry to %1...").arg(ai.model));
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    ui->statusBar->clearMessage();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        parseResponse(response);
    } else {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray responseData = reply->readAll();

        switch(httpCode)
        {
            case DEEPSEEK_ERROR_CODES_INVALID_FORMAT:           // Invalid Format
                ui->te_ChatHistory->append("⚠️ Invalid Format: Invalid request body format.");
                ui->te_ChatHistory->append("💡 Solution: Please modify your request body according to the hints in the error message.\nFor more API format details, please refer to DeepSeek API Docs.");

                // Предложите альтернативу
                suggestAlternative();
            break;
            case DEEPSEEK_ERROR_CODES_AUTHENTICATION_FAILS:     // Authentication Fails
                ui->te_ChatHistory->append("⚠️ Authentication Fails: Authentication fails due to the wrong API key.");
                ui->te_ChatHistory->append("💡 Solution: Please check your API key. If you don't have one, please create an API key first.");

                // Предложите альтернативу
                suggestAlternative();
                break;
            case DEEPSEEK_ERROR_CODES_INSUFFICIENT_BALANCE:
                ui->te_ChatHistory->append("⚠️ Ошибка баланса: Недостаточно средств на счете API.");
                ui->te_ChatHistory->append("💡 Solution: Пополните баланс на platform.deepseek.com");

                // Предложите альтернативу
                suggestAlternative();
            break;
            case DEEPSEEK_ERROR_CODES_INVALID_PARAMETERS:
                ui->te_ChatHistory->append("⚠️ Invalid request parameters: Your request contains invalid parameters.");
                ui->te_ChatHistory->append("💡 Solution: Please modify your request parameters according to the hints in the error message.\nFor more API format details, please refer to DeepSeek API Docs.");

                // Предложите альтернативу
                suggestAlternative();
            break;
            case DEEPSEEK_ERROR_CODES_RATE_LIMIT_REACHED:
                ui->te_ChatHistory->append("⚠️ Request rate limit exceeded: You are sending requests too quickly.");
                ui->te_ChatHistory->append("💡 Solution: Please pace your requests reasonably.\nWe also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.");

                // Предложите альтернативу
                suggestAlternative();
            break;
            case DEEPSEEK_ERROR_CODES_SERVER_ERROR:
                ui->te_ChatHistory->append("⚠️ Internal server error: Our server encounters an issue.");
                ui->te_ChatHistory->append("💡 Solution: Please retry your request after a brief wait and contact us if the issue persists.");

                // Предложите альтернативу
                suggestAlternative();
            break;
            case DEEPSEEK_ERROR_CODES_SERVER_OVERLOADED:
                ui->te_ChatHistory->append("⚠️ Server overloaded due to high traffic: The server is overloaded due to high traffic.");
                ui->te_ChatHistory->append("💡 Solution: Please retry your request after a brief wait.");

                // Предложите альтернативу
                suggestAlternative();
            break;
            default:
                ui->te_ChatHistory->append("Ошибка: " + reply->errorString());
            break;
        }
    }
    reply->deleteLater();
}

void MainWindow::suggestAlternative()
{
    QString message = ui->te_Message->toPlainText();
    ui->te_ChatHistory->append("🤖 Локальный ответ: Привет! Сейчас я не могу подключиться к DeepSeek API из-за недостатка баланса. "
                               "Пожалуйста, пополните счет на platform.deepseek.com чтобы продолжить использование нейросети.");
}

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

                ui->te_ChatHistory->append("DeepSeek: " + content);
                return;
            }
        }

        if (json.contains("error"))
        {
            QJsonObject error = json["error"].toObject();
            QString errorMsg = error["message"].toString();
            ui->te_ChatHistory->append("Ошибка API: " + errorMsg);
        }
    } else {
        ui->te_ChatHistory->append("Ошибка парсинга ответа");
    }
}
