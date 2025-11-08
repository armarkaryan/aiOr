#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pb_Send_clicked();
    void onReplyFinished(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QString apiKey = "your_deepseek_api_key_here"; // Замените на ваш API ключ

    void sendMessageToDeepSeek(const QString &message);
    void parseResponse(const QByteArray &response);
};
#endif
