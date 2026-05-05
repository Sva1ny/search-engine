#include <QApplication>
#include "widget.h"
#include "connectiondialog.h"
#include <QMessageBox>
#include <QGuiApplication> // 新增头文件

void handleConnected(QTcpSocket *tcpSocket, ConnectionDialog &connectionDialog)
{
    Widget *w = new Widget(tcpSocket);
    connectionDialog.close();
    w->show();

    // 清理 Widget 实例
    QObject::connect(w, &QWidget::destroyed, [&]()
                     { delete w; });
}

int main(int argc, char *argv[])
{
    qputenv("QT_LOGGING_RULES", "qt.debug=true");
    // 设置XCB为平台插件
    qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication a(argc, argv);

    ConnectionDialog connectionDialog;

    // 连接信号到处理函数
    QObject::connect(&connectionDialog, &ConnectionDialog::connected, [&](QTcpSocket *tcpSocket)
                     {
        if (tcpSocket) 
        {
            handleConnected(tcpSocket, connectionDialog);
        } 
        else 
        {
            QMessageBox::critical(&connectionDialog, "连接失败", "无法连接到服务器，请检查 IP 地址和端口号。");
            connectionDialog.show();  // 显示 ConnectionDialog 以便重新连接
        } });

    connectionDialog.show();

    return a.exec();
}
