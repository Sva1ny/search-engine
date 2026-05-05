#include "EventLoop.h"
#include "TcpConnection.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>

EventLoop::EventLoop(Acceptor &acceptor)
    : _epfd(createEpollFd()),
      _isLooping(false),
      _evtList(1024),
      _acceptor(acceptor),
      _evtfd(createEventFd()),
      _mutex()
{
    // 将listenfd放在红黑树上进行监听
    int listenfd = _acceptor.fd();
    addEpollReadFd(listenfd);
    // 将用于通信的文件描述符进行监听
    addEpollReadFd(_evtfd);
}

EventLoop::~EventLoop()
{
    close(_epfd);
    close(_evtfd);
}

void EventLoop::loop()
{
    _isLooping = true;
    while (_isLooping)
    {
        waitEpollFd();
    }
}

void EventLoop::unloop()
{
    _isLooping = false;
}

// 封装类epoll_wait函数
void EventLoop::waitEpollFd()
{
    int ready_num = 0;
    do
    {
        ready_num = epoll_wait(_epfd, &(*_evtList.begin()), _evtList.size(), 3000);
    } while (-1 == ready_num && errno == EINTR);

    if (-1 == ready_num)
    {
        return;
    }
    else if (0 == ready_num)
    {
        std::cout << ">> epoll timeout" << std::endl;
    }
    else
    {
        if (ready_num == (int)_evtList.size())
        {
            _evtList.resize(2 * ready_num);
        }

        for (int idx = 0; idx < ready_num; ++idx)
        {
            // 连接是listenfd
            int fd = _evtList[idx].data.fd;
            int listenfd = _acceptor.fd();
            if (fd == listenfd)
            {
                if (_evtList[idx].events & EPOLLIN)
                {
                    // 处理新的连接
                    handleNewConnection();
                }
            }
            // 监听的用于通信的文件描述符就绪了
            else if (fd == _evtfd)
            {
                if (_evtList[idx].events & EPOLLIN)
                {
                    handleRead();
                    // 执行所有的"任务"
                    doPengdingFunctors();
                }
            }
            else // 处理老的连接
            {
                if (_evtList[idx].events & EPOLLIN)
                {
                    handleMessage(fd);
                }
            }
        }
    }
}

// 处理新的连接
void EventLoop::handleNewConnection()
{
    int connfd = _acceptor.accept();
    addEpollReadFd(connfd);

    TcpConnectionPtr tc(new TcpConnection(connfd, this));

    tc->setNewConnectionCallback(_onNewConnectionCb);
    tc->setMessageCallback(_onMessageCb);
    tc->setCloseCallback(_onCloseCb);
    _conns[connfd] = tc;

    tc->handleNewConnectionCallback();
}

// 处理老的连接上的消息
void EventLoop::handleMessage(int fd)
{
    auto it = _conns.find(fd);
    if (it != _conns.end())
    {
        bool flag = it->second->isClosed();
        if (flag)
        {
            it->second->handleCloseCallback();
            delEpollReadFd(fd);
            _conns.erase(it);
        }
        else
        {
            it->second->handleMessageCallback();
        }
    }
    else
    {
        return;
    }
}

// epfd的创建
int EventLoop::createEpollFd()
{
    return ::epoll_create(1);
}

// 监听文件描述符
void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    ::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
}

// 取消文件描述符的监听
void EventLoop::delEpollReadFd(int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void EventLoop::setNewConnectionCallback(TcpConnectionCallback &&cb)
{
    _onNewConnectionCb = std::move(cb);
}

void EventLoop::setMessageCallback(TcpConnectionCallback &&cb)
{
    _onMessageCb = std::move(cb);
}

void EventLoop::setCloseCallback(TcpConnectionCallback &&cb)
{
    _onCloseCb = std::move(cb);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t ret = read(_evtfd, &one, sizeof(uint64_t));
}

// 将存放在vector中的"任务"进行遍历执行
void EventLoop::doPengdingFunctors()
{
    vector<Functor> tmp;
    {
        _mutex.lock();
        tmp.swap(_pendings);
        _mutex.unlock();
    }
    // 将所有的任务都进行执行
    for (auto &cb : tmp)
    {
        cb(); // 回调的执行
    }
}

int EventLoop::createEventFd()
{
    int fd = eventfd(10, 0);
    if (fd < 0)
    {
        std::cout << ">> create eventfd error" << std::endl;
        return fd;
    }
    return fd;
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t ret = write(_evtfd, &one, sizeof(uint64_t));
}

void EventLoop::runInLoop(Functor &&cb)
{
    {
        _mutex.lock();
        _pendings.push_back(std::move(cb));
        _mutex.unlock();
    }
    // 线程池就需要通知EventLoop执行“任务”
    wakeup();
}