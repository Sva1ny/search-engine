#ifndef SEARCHENGINESERVER_H
#define SEARCHENGINESERVER_H

#include "ThreadPool.h"
#include "TcpServer.h"
#include "KeyRecommander.h"
#include "WebPageSearcher.h"
#include <memory>
using namespace std;

class MyTask
{
public:
    MyTask(const string &msg, const TcpConnectionPtr &coon);
    void process();

private:
    string _msg;
    TcpConnectionPtr _conn;
};

class SearchEngineServer
{
public:
    SearchEngineServer(size_t threadNum, size_t queSize, const string &ip, uint16_t port);
    ~SearchEngineServer();
    void start();
    void stop();
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn);
    void onClose(const TcpConnectionPtr &conn);
    void doTaskThread(const TcpConnectionPtr &conn, string &jsonmsg);
    void warmUpCaches();

private:
    ThreadPool _pool;
    TcpServer _server;
    KeyRecommander _recommander;
    WebPageSearcher _searcher;
};

#endif