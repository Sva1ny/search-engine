#include "SocketIO.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <unistd.h>
SocketIO::SocketIO(int fd)
    : _fd(fd)
{
}

SocketIO::~SocketIO()
{
    close(_fd);
}

int SocketIO::readn(char *buf, int len)
{
    size_t total_received = 0;
    while (total_received < len)
    {
        int once_received = recv(_fd, buf + total_received, len - total_received, MSG_NOSIGNAL);
        if (once_received == -1 && errno == EINTR)
        {
            continue;
        }
        else if (once_received == -1)
        {
            return -1;
        }
        else if (once_received == 0)
        {
            break;
        }
        total_received += once_received;
    }

    return total_received;
}

int SocketIO::readLine(char *buf, int len)
{
    int left = len - 1;
    char *pstr = buf;
    int ret = 0, total = 0;

    while (left > 0)
    {
        // MSG_PEEK不会将缓冲区中的数据进行清空,只会进行拷贝操作
        ret = recv(_fd, pstr, left, MSG_PEEK);
        if (-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if (-1 == ret)
        {
            return -1;
        }
        else if (0 == ret)
        {
            break;
        }
        else
        {
            for (int idx = 0; idx < ret; ++idx)
            {
                if (pstr[idx] == '\n')
                {
                    int sz = idx + 1;
                    readn(pstr, sz);
                    pstr += sz;
                    *pstr = '\0'; // C风格字符串以'\0'结尾

                    return total + sz;
                }
            }

            readn(pstr, ret); // 从内核态拷贝到用户态
            total += ret;
            pstr += ret;
            left -= ret;
        }
    }
    *pstr = '\0';

    return total - left;
}

int SocketIO::writen(const char *buf, int len)
{
    send(_fd, &len, sizeof(int), MSG_NOSIGNAL);
    std::size_t total_sent = 0;
    while (total_sent < len)
    {
        int once_sent = send(_fd, buf + total_sent, len - total_sent, MSG_NOSIGNAL);
        if (once_sent == -1 && errno == EINTR)
        {
            continue;
        }
        else if (once_sent == -1)
        {
            return -1;
        }
        else if (once_sent == 0)
        {
            break;
        }
        total_sent += once_sent;
    }
    return total_sent;
}