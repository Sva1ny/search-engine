#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "Acceptor.h"
#include "EventLoop.h"

class TcpServer
{
public:
    TcpServer(const string &ip, unsigned short port);
    ~TcpServer();

    // 服务器的开始与停止
    void start();
    void stop();
    void setAllCallback(TcpConnectionCallback &&cb1, TcpConnectionCallback &&cb2, TcpConnectionCallback &&cb3);

private:
    Acceptor _acceptor;
    EventLoop _loop;
};

#endif
