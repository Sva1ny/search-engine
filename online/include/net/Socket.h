#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "NoCopyable.h"

class Socket
    : NoCopyable
{
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();
    int fd() const;
    void shutDownWrite();

private:
    int _fd;
};

#endif
