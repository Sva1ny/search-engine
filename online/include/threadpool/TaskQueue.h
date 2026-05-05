#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

using std::condition_variable;
using std::function;
using std::mutex;
using std::queue;
using std::size_t;

using ElemType = function<void()>;

class TaskQueue
{
public:
    TaskQueue(size_t queSize);
    ~TaskQueue();
    // 添加任务
    void push(ElemType &&);
    // 获取任务
    ElemType pop();

    // 任务队列是空与满
    bool full() const;
    bool empty() const;
    void wakeup();

private:
    size_t _queSize;              // 任务队列的大小
    queue<ElemType> _que;         // 存放任务的数据结构
    mutex _mutex;                 // 互斥锁
    condition_variable _notFull;  // 非满条件变量（生产者）
    condition_variable _notEmpty; // 非空条件变量（消费者）
    bool _flag;
};

#endif
