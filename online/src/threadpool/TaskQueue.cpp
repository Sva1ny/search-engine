#include "TaskQueue.h"

using std::unique_lock;

TaskQueue::TaskQueue(size_t queSize)
    : _queSize(queSize), _flag(true)
{
}

TaskQueue::~TaskQueue() {}
// 添加任务
void TaskQueue::push(ElemType &&task)
{
    unique_lock<mutex> lock(_mutex);
    while (full())
    {
        _notFull.wait(lock);
    }
    _que.push(std::move(task));
    _notEmpty.notify_one();
}
// 获取任务
ElemType TaskQueue::pop()
{
    unique_lock<mutex> lock(_mutex);
    while (empty() && _flag)
    {
        _notEmpty.wait(lock);
    }
    if (!_flag)
    {
        return ElemType();
    }
    ElemType ret = _que.front();
    _que.pop();
    _notFull.notify_one();
    return ret;
}

// 任务队列是空与满
bool TaskQueue::full() const
{
    return _que.size() == _queSize;
}

bool TaskQueue::empty() const
{
    return 0 == _que.size();
}

void TaskQueue::wakeup()
{
    _flag = false;
    _notEmpty.notify_all();
}