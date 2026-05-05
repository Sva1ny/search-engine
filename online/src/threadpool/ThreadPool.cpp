#include "ThreadPool.h"
#include <unistd.h>
#include <iostream>

using std::cout;
using std::endl;

ThreadPool::ThreadPool(size_t threadNum, size_t queSize, size_t cacheSize)
    : _threadNum(threadNum),
      _queSize(queSize),
      _taskQue(_queSize),
      _isExit(false),
      _mainkeyCache(cacheSize * 2),
      _mainpageCache(cacheSize * 2),
      _syncRunning(false)
{
    _threads.reserve(_threadNum);
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::start()
{
    // 启动所有工作线程
    for (size_t i = 0; i < _threadNum; ++i)
    {
        _threadData.push_back(new ThreadLocalData(_mainkeyCache.capacity() / 2));
        _threads.emplace_back([this, i]()
                              {
            _threadData[i]->owner_thread_id = std::this_thread::get_id();
            doTask(); });
    }

    // 启动定时同步线程
    _timerRunning = true;
    _timerThread.reset(new std::thread([this]()
                                       {
        while (_timerRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
            syncCaches();
        } }));
}

void ThreadPool::stop()
{
    // 停止定时同步线程
    _timerRunning = false;
    if (_timerThread && _timerThread->joinable())
    {
        _timerThread->join();
    }

    while (!_taskQue.empty())
    {
        sleep(1);
    }

    _isExit = true;

    _taskQue.wakeup();

    for (auto &th : _threads)
    {
        th.join();
    }
}

void ThreadPool::addTask(Task &&task)
{
    if (task)
    {
        _taskQue.push(std::move(task));
    }
}

void ThreadPool::doTask()
{
    while (!_isExit)
    {

        Task task = getTask();
        if (task)
        {
            {
                std::lock_guard<std::mutex> mainLock(_mainCacheMutex);
                for (auto *data : _threadData)
                {
                    std::lock_guard<std::mutex> lock(data->cache_mutex);
                    // 同步key缓存
                    for (const auto &item : _mainkeyCache.getAll())
                    {
                        data->keycache.put(item.first, item.second);
                    }
                    // 同步page缓存
                    for (const auto &item : _mainpageCache.getAll())
                    {
                        data->pagecache.put(item.first, item.second);
                    }
                }
            }
            task(); // 执行任务
        }
        else
        {
            cout << "nullptr == ptask" << endl;
        }
    }
}

Task ThreadPool::getTask()
{
    return _taskQue.pop();
}

void ThreadPool::addToMainKeyCache(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(_mainCacheMutex);
    _mainkeyCache.put(key, value);
}
void ThreadPool::addToMainPageCache(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(_mainCacheMutex);
    _mainpageCache.put(key, value);
}
// 添加缓存同步方法
void ThreadPool::syncCaches()
{
    if (_syncRunning.exchange(true))
        return;

    for (auto *data : _threadData)
    {
        std::lock_guard<std::mutex> lock(data->patch_mutex);
        {
            std::lock_guard<std::mutex> mainLock(_mainCacheMutex);
            for (auto &item : data->patch.keypatch)
            {
                _mainkeyCache.put(item.first, item.second);
            }
            for (auto &item : data->patch.pagepatch)
            {
                _mainpageCache.put(item.first, item.second);
            }
        }
        data->patch.keypatch.clear();
        data->patch.pagepatch.clear();
    }
    cout << "Cache synchronized" << endl;
    // 将主缓存复制到各线程缓存
    // 这里简化处理，实际可能需要更复杂的同步机制
    _syncRunning = false;
}
// 获取当前线程的本地数据
vector<ThreadLocalData *> ThreadPool::getThreadData() const
{
    return _threadData;
}