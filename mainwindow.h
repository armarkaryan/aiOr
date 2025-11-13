#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include "deepseek_error_codes.h"
#include "api_key_reader.h"

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
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void on_pb_Send_clicked();
    void onReplyFinished(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    struct AI
    {
        QString model = "deepseek-coder";                              //!<
        QString url = "https://api.deepseek.com/v1/chat/completions";  //!<
        QString apiKey = "your_deepseek_api_key_here";                 //!< AI API-key
        QString max_tokens = "4000";                                   //!<
        QString temperature = "0.3";                                   //!<
        QString stream = "false";                                      //!<
    }ai;

    void sendMessageToAI(const QString &message);
    void suggestAlternative();
    void parseResponse(const QByteArray &response);
};
#endif /* _MAINWINDOW_H_ */
