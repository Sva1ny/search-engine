#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QDesktopServices>
#include <QUrl>
#include <QListWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QTcpSocket *tcpSocket, QWidget *parent = nullptr);
    ~Widget();

private slots:
    void connected_Slot();
    void readyRead_Slot();
    void keyWordRecommend();
    void on_searchBt_clicked();
    void handleLinkClicked(const QUrl &url);
    void on_prevPageBt_clicked();
    void on_nextPageBt_clicked();
    void on_jumpBt_clicked();
    void handlePaginationLinks(const QUrl &url);
    void do_recommendItem_clicked(QListWidgetItem *item);
    void sendRecommendRequest(); // 新增的槽函数声明

private:
    Ui::Widget *ui;
    QTcpSocket *tcpSocket;
    QJsonArray searchResults;
    int currentPage;
    int itemsPerPage;
    int maxPage;
    QJsonArray queryWords;
    QTimer *recommendTimer; // 新增的QTimer成员变量
    void displaySearchResults();
    void sendSearchRequest(int page);
};

#endif // WIDGET_H
