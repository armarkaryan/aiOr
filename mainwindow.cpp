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

    setWindowTitle("aiOr - DeepSeek Chat Client");
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
        sendMessageToDeepSeek(message);
    }
}

void MainWindow::sendMessageToDeepSeek(const QString &message)
{
    QUrl url("https://api.deepseek.com/v1/chat/completions");

    QNetworkRequest request(url);

    // SSL конфигурация
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    request.setSslConfiguration(sslConfig);

    // Заголовки
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(apiKey).toUtf8());

    // JSON данные
    QJsonObject json;
    json["model"] = "deepseek-chat";

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = 1000;
    json["temperature"] = 0.7;
    json["stream"] = false;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // Отправка
    networkManager->post(request, data);
    ui->statusBar->showMessage("Отправка запроса...");
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
            case 402:
                ui->te_ChatHistory->append("⚠️ Ошибка баланса: Недостаточно средств на счете API");
                ui->te_ChatHistory->append("💡 Решение: Пополните баланс на platform.deepseek.com");

                // Предложите альтернативу
                suggestAlternative();
            break;
            default:
                ui->te_ChatHistory->append("Ошибка: " + reply->errorString());
            break;
        }
/*
        if (httpCode == 402) {
            ui->te_ChatHistory->append("⚠️ Ошибка баланса: Недостаточно средств на счете API");
            ui->te_ChatHistory->append("💡 Решение: Пополните баланс на platform.deepseek.com");

            // Предложите альтернативу
            suggestAlternative();
        } else {
            ui->te_ChatHistory->append("Ошибка: " + reply->errorString());
        }
*/
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
