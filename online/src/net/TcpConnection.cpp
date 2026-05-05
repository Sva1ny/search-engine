#include "TcpConnection.h"
#include "EventLoop.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

TcpConnection::TcpConnection(int fd, EventLoop *loop)
    : _loop(loop), _sockIO(fd), _sock(fd), _localAddr(getLocalAddr()), _peerAddr(getPeerAddr())
{
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::send(const string &msg)
{
    _sockIO.writen(msg.c_str(), msg.size());
}

void TcpConnection::sendInLoop(const string &msg)
{
    if (_loop)
    {
        _loop->runInLoop(bind(&TcpConnection::send, this, msg));
    }
}

string TcpConnection::receive()
{
    char buf[65535];
    _sockIO.readLine(buf, sizeof(buf));
    return string(buf);
}

bool TcpConnection::isClosed() const
{
    char buf[10] = {0};
    int ret = ::recv(_sock.fd(), buf, sizeof(buf), MSG_PEEK);

    return (0 == ret);
}

string TcpConnection::toString()
{
    return _localAddr.ip() + ":" + std::to_string(_localAddr.port()) + "-->" + _peerAddr.ip() + ":" + std::to_string(_peerAddr.port());
}

void TcpConnection::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    _onNewConnectionCb = cb;
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback &cb)
{
    _onMessageCb = cb;
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback &cb)
{
    _onCloseCb = cb;
}

void TcpConnection::handleNewConnectionCallback()
{
    if (_onNewConnectionCb)
    {
        _onNewConnectionCb(shared_from_this());
    }
}

void TcpConnection::handleMessageCallback()
{
    if (_onMessageCb)
    {
        _onMessageCb(shared_from_this());
    }
}

void TcpConnection::handleCloseCallback()
{
    if (_onCloseCb)
    {
        _onCloseCb(shared_from_this());
    }
}

InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    // 获取本端地址的函数getsockname
    int ret = getsockname(_sock.fd(), (struct sockaddr *)&addr, &len);
    if (-1 == ret)
    {
        perror("getsockname");
    }

    return InetAddress(addr);
}

InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    // 获取对端地址的函数getpeername
    int ret = getpeername(_sock.fd(), (struct sockaddr *)&addr, &len);
    if (-1 == ret)
    {
        perror("getpeername");
    }

    return InetAddress(addr);
}