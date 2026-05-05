#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QMessageBox>
#include <QTimer>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog),
    tcpSocket(nullptr),
    timeoutTimer(new QTimer(this)),
    checkTimer(new QTimer(this)),
    countdownLabel(new QLabel(this))
{
    ui->setupUi(this);
    setWindowTitle("连接搜索引擎");

    // 设置倒计时标签属性
    countdownLabel->setStyleSheet("font-size: 16px; color: red;");
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setGeometry(10, 10, 200, 30);  // 设置位置和大小
    countdownLabel->hide();  // 初始时隐藏倒计时标签

    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectionDialog::onConnectClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &ConnectionDialog::onTimeout);
    connect(checkTimer, &QTimer::timeout, this, &ConnectionDialog::checkConnectionStatus);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::onConnectClicked()
{
    QString ipAddress = ui->ipEdit->text();
    quint16 port = ui->portEdit->text().toUShort();

    // 创建 QTcpSocket 实例
    tcpSocket = new QTcpSocket(this);

    // 设置连接超时定时器
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(2000);  // 2秒超时

    // 设置检查连接状态的定时器
    checkTimer->setInterval(100);  // 每100毫秒检查一次
    checkTimer->start();

    // 开始连接到主机
    tcpSocket->connectToHost(ipAddress, port);

    // 连接成功处理
    connect(tcpSocket, &QTcpSocket::connected, this, &ConnectionDialog::onConnected);

    // 显示倒计时标签
    showCountdown(2);  // 2秒倒计时
}

void ConnectionDialog::onConnected()
{
    // 停止定时器
    timeoutTimer->stop();
    checkTimer->stop();

    emit connected(tcpSocket);
    this->accept();  // 关闭 ConnectionDialog 窗口
}

void ConnectionDialog::onTimeout()
{
    // 连接超时处理
    if (tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::critical(this, "连接超时", "连接超时，请检查网络和主机状态。");
        tcpSocket->abort();  // 中止连接
        delete tcpSocket;
        tcpSocket = nullptr;
    }
}

void ConnectionDialog::checkConnectionStatus()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        onConnected();
    }
}

void ConnectionDialog::showCountdown(int seconds)
{
    countdownLabel->setText(QString("倒计时: %1秒").arg(seconds));
    countdownLabel->show();
    QTimer::singleShot(seconds * 1000, this, [this]() {
        countdownLabel->hide();
    });
}
