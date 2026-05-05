#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtEndian> // 用于字节序转换

Widget::Widget(QTcpSocket *tcpSocket, QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget), tcpSocket(tcpSocket), currentPage(1), itemsPerPage(5), maxPage(0), recommendTimer(new QTimer(this))
{
    ui->setupUi(this);
    setWindowTitle("SearchEngine");
    if (!tcpSocket)
    {
        // 处理空的 tcpSocket 情况
        QMessageBox::critical(this, "连接错误", "未能获取有效的 TCP 连接！");
        this->close();
        return;
    }

    // 连接成功后的信号槽
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));
    ui->recommendListWidget->hide();
    // 其他信号槽连接
    connect(ui->receiveEdit, &QTextBrowser::anchorClicked, this, &Widget::handleLinkClicked);
    connect(ui->sendEdit, &QLineEdit::textChanged, this, &Widget::keyWordRecommend);
    connect(ui->recommendListWidget, &QListWidget::itemClicked, this, &Widget::do_recommendItem_clicked);

    // 设置QTimer
    recommendTimer->setSingleShot(true);
    connect(recommendTimer, &QTimer::timeout, this, &Widget::sendRecommendRequest);
}

Widget::~Widget()
{
    delete ui;
    if (tcpSocket)
    {
        tcpSocket->disconnectFromHost();
        tcpSocket->deleteLater();
    }
}

void Widget::connected_Slot()
{
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));
}

void Widget::readyRead_Slot()
{
    static QByteArray buffer;
    static quint32 expectedLength = 0;

    while (tcpSocket->bytesAvailable())
    {
        if (expectedLength == 0)
        {
            if (tcpSocket->bytesAvailable() < sizeof(quint32))
            {
                return;
            }

            // 读取原始长度数据
            QByteArray lengthData = tcpSocket->read(sizeof(quint32));
            qDebug() << "Raw length data (hex):" << lengthData.toHex();

            // 使用小端字节序转换
            expectedLength = qFromLittleEndian<quint32>(
                reinterpret_cast<const uchar *>(lengthData.constData()));

            qDebug() << "Expected length:" << expectedLength;

            // 长度有效性检查
            if (expectedLength > 10 * 1024 * 1024)
            {
                qDebug() << "Invalid length received:" << expectedLength;
                tcpSocket->abort();
                expectedLength = 0;
                return;
            }
        }

        if (tcpSocket->bytesAvailable() < expectedLength)
        {
            return; // 等待更多数据到来
        }

        buffer.append(tcpSocket->read(expectedLength));
        qDebug() << "Received raw data:" << buffer; // 添加调试输出
        expectedLength = 0;

        QJsonDocument doc = QJsonDocument::fromJson(buffer);
        if (doc.isNull())
        {
            qDebug() << "Invalid JSON data:" << buffer;
            buffer.clear();
            return;
        }
        buffer.clear();

        QJsonObject jsonObj = doc.object();
        qDebug() << "Parsed JSON:" << jsonObj; // 添加调试输出
        int msgID = jsonObj["msgID"].toInt();
        if (msgID == 1)
        {
            ui->recommendListWidget->clear(); // 清空之前的推荐词
            QJsonArray keywords = jsonObj["msg"].toArray();
            for (const QJsonValue &value : keywords)
            {
                QString keyword = value.toString();
                ui->recommendListWidget->addItem(keyword); // 添加推荐词到列表
            }
            if (keywords.size() > 0)
            {
                ui->recommendListWidget->show();  // 显示推荐词列表
                ui->recommendListWidget->raise(); // 置于顶层
            }
        }
        else if (msgID == 2)
        {
            ui->recommendListWidget->hide();
            maxPage = 3;
            searchResults = jsonObj["files"].toArray();
            // queryWords = jsonObj["queryWords"].toArray();
            displaySearchResults();
        }
        else
        {
            ui->receiveEdit->setHtml("<p style='color:orange;'>Unexpected msgID: " + QString::number(msgID) + "</p>");
        }
    }
}

void Widget::displaySearchResults()
{
    QString htmlContent;
    int totalItems = searchResults.size();

    // 清理HTML标签的正则表达式
    QRegularExpression cleanTags("<[^>]*>");

    for (int i = 0; i < totalItems; ++i)
    {
        QJsonObject fileObj = searchResults[i].toObject();
        QString title = fileObj["title"].toString().remove(cleanTags).trimmed();
        QString summary = fileObj["abstract"].toString().remove(cleanTags).trimmed();
        QString url = fileObj["url"].toString().remove(cleanTags).trimmed();

        // 标记 summary 中的 queryWords，增加背景色
        for (const QJsonValue &value : queryWords)
        {
            QString queryWord = value.toString();
            summary.replace(queryWord, "<span style='color:red; background-color:yellow;'>" + queryWord + "</span>", Qt::CaseInsensitive);
        }

        htmlContent.append(
            "<div>"
            "<p><b>[标题]</b> <a href='" +
            url + "' style='color:green;'>" + title + "</a></p>"
                                                      "<p><b>[摘要]</b> " +
            summary + "</p>"
                      "<p><b>[链接]</b> <a href='" +
            url + "'>" + url + "</a></p>"
                               "<hr/>"
                               "</div>");
    }

    htmlContent.append("<p>当前页码: " + QString::number(currentPage) + " / 总页码: " + QString::number(3) + "</p>");

    ui->receiveEdit->setHtml(htmlContent);
}

void Widget::keyWordRecommend()
{
    if (ui->sendEdit->text().isEmpty())
    {
        ui->recommendListWidget->hide();
        return;
    }

    recommendTimer->start(500);
}

void Widget::sendRecommendRequest()
{
    if (ui->sendEdit->text().isEmpty())
    {
        ui->recommendListWidget->hide();
        return;
    }

    int query_id = 1;
    QString msg = ui->sendEdit->text();

    QJsonObject json;
    json["query_id"] = query_id;
    json["msg"] = msg;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact) + "\n";

    tcpSocket->write(jsonData);
}

void Widget::on_searchBt_clicked()
{
    searchResults = QJsonArray();
    currentPage = 1;
    ui->recommendListWidget->hide();
    sendSearchRequest(currentPage);
}

void Widget::sendSearchRequest(int page)
{
    int query_id = 2;
    QString msg = ui->sendEdit->text();

    QJsonObject json;
    json["query_id"] = query_id;
    json["msg"] = msg;
    json["pageNum"] = page;
    json["itemsPerPage"] = itemsPerPage;

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact) + "\n";

    tcpSocket->write(jsonData);
}

void Widget::handleLinkClicked(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

void Widget::on_prevPageBt_clicked()
{
    if (currentPage > 1)
    {
        sendSearchRequest(--currentPage);
    }
}

void Widget::on_nextPageBt_clicked()
{
    if (currentPage < maxPage)
    {
        sendSearchRequest(++currentPage);
    }
}

void Widget::on_jumpBt_clicked()
{
    bool ok;
    int page = ui->pageEdit->text().toInt(&ok);
    if (ok)
    {
        if (page >= 1 && page <= maxPage)
        {
            currentPage = page;
            sendSearchRequest(page);
        }
        else
        {
            QMessageBox::warning(this, "页码错误", "页码不合法，请输入有效的页码！");
        }
    }
    else
    {
        QMessageBox::warning(this, "输入错误", "请输入有效的数字页码！");
    }
}

void Widget::handlePaginationLinks(const QUrl &url)
{
    if (url.fragment() == "prevPage")
    {
        on_prevPageBt_clicked();
    }
    else if (url.fragment() == "nextPage")
    {
        on_nextPageBt_clicked();
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

void Widget::do_recommendItem_clicked(QListWidgetItem *item)
{
    QString keyword = item->text();

    // 临时断开信号连接
    disconnect(ui->sendEdit, &QLineEdit::textChanged, this, &Widget::keyWordRecommend);
    ui->sendEdit->setText(keyword);
    // 重新连接信号
    connect(ui->sendEdit, &QLineEdit::textChanged, this, &Widget::keyWordRecommend);

    on_searchBt_clicked(); // 发起搜索操作
}
