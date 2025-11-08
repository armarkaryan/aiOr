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

    // Подключаем сигнал завершения запроса
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);

    // Настройка интерфейса
    setWindowTitle("aiOr - DeepSeek Chat Client");
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
    QUrl url("https://api.deepseek.com/v1/chat/completions"); // Проверьте актуальный URL API
    //QUrl url("https://api.deepseek.com/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // Формируем JSON запрос
    QJsonObject json;
    json["model"] = "deepseek-chat"; // Уточните модель в документации

    QJsonArray messages;
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = message;
    messages.append(messageObj);

    json["messages"] = messages;
    json["max_tokens"] = 1000;
    json["temperature"] = 0.7;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // Отправляем POST запрос
    networkManager->post(request, data);

    ui->statusBar->showMessage("Отправка запроса...");
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    ui->statusBar->clearMessage();

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        parseResponse(response);
    } else {
        ui->te_ChatHistory->append("Ошибка: " + reply->errorString());
        QMessageBox::warning(this, "Ошибка сети", reply->errorString());
    }

    reply->deleteLater();
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
