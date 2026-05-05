#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();

signals:
    void connected(QTcpSocket *);  // 连接成功的信号，传递QTcpSocket对象

private slots:
    void onConnectClicked();  // 处理连接按钮点击事件
    void onConnected();      // 处理连接成功的槽
    void onTimeout();        // 处理连接超时的槽
    void checkConnectionStatus();  // 定期检查连接状态

private:
    Ui::ConnectionDialog *ui;  // 使用UI类
    QTcpSocket *tcpSocket;     // 连接用的QTcpSocket
    QTimer *timeoutTimer;      // 超时定时器
    QTimer *checkTimer;        // 检查连接状态的定时器
    QLabel *countdownLabel;    // 倒计时标签

    void showCountdown(int seconds);  // 显示倒计时窗口
};

#endif // CONNECTIONDIALOG_H
