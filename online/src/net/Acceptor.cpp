#include "Acceptor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

Acceptor::Acceptor(const string &ip, unsigned short port)
    : _sock(), _addr(ip, port)
{
}

Acceptor::~Acceptor()
{
}

void Acceptor::ready()
{
    setReuseAddr();
    setReusePort();
    bind();
    listen();
}

void Acceptor::setReuseAddr()
{
    int reuse = 1;
    int ret = ::setsockopt(_sock.fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}

void Acceptor::setReusePort()
{
    int reuse = 1;
    int ret = ::setsockopt(_sock.fd(), SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
}

void Acceptor::bind()
{
    ::bind(_sock.fd(), (struct sockaddr *)_addr.getInetAddrPtr(), sizeof(struct sockaddr));
}

void Acceptor::listen()
{
    ::listen(_sock.fd(), 1024);
}

int Acceptor::accept()
{
    int connfd = ::accept(_sock.fd(), nullptr, nullptr);
    return connfd;
}

int Acceptor::fd() const
{
    return _sock.fd();
}
