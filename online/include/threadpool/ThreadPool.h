#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "LRUCache.h"
#include "TaskQueue.h"
#include <thread>
#include <memory>
#include <functional>
#include <vector>
#include <atomic>

using std::function;
using std::shared_ptr;
using std::thread;
using std::vector;

using Task = function<void()>;

class ThreadPool
{
public:
    ThreadPool(size_t threadNum, size_t queSize, size_t cacheSize = 10000);
    ~ThreadPool();

    // 线程池的启动与停止
    void start();
    void stop();

    // 添加任务与获取任务
    void addTask(Task &&ptask);
    std::vector<ThreadLocalData *> getThreadData() const;
    void addToMainKeyCache(const std::string &key, const std::string &value);
    void addToMainPageCache(const std::string &key, const std::string &value);

private:
    Task getTask();
    // 线程池交给工作线程执行的任务
    void doTask();

private:
    size_t _threadNum;       // 子线程的数目
    size_t _queSize;         // 任务队列的大小
    vector<thread> _threads; // 存放工作线程的容器
    TaskQueue _taskQue;      // 存放任务的容器
    bool _isExit;            // 标志线程池是否结束的标志位

    LRUCache<std::string, std::string> _mainkeyCache;
    LRUCache<std::string, std::string> _mainpageCache;
    std::vector<ThreadLocalData *> _threadData;
    std::mutex _mainCacheMutex;

    std::unique_ptr<std::thread> _timerThread; // 使用线程代替Timer
    std::atomic<bool> _timerRunning;
    std::atomic<bool> _syncRunning;

    // 添加新方法
    void syncCaches();
};

#endif
