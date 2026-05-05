#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket()
    : _fd(socket(AF_INET, SOCK_STREAM, 0))
{
}

Socket::Socket(int fd)
    : _fd(fd)
{
}

Socket::~Socket()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

int Socket::fd() const
{
    return _fd;
}

void Socket::shutDownWrite()
{
    shutdown(_fd, SHUT_WR);
}