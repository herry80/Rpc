#pragma once
#include<queue>
#include<thread>
#include<mutex>//互斥锁
#include<condition_variable>//条件变量
using namespace std;
//异步写日志的日志队列
template<class T>
class LockQueue
{
    private:
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_condvariable;

    public:
    void Push(const T&data)
    {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();//只有一个线程去写入文件，所以唤醒一个线程
    }
    T Pop()
    {
        unique_lock<mutex>lock(m_mutex);
        while(m_queue.empty())
        {
            //日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
        }
        //不为空
        T data=m_queue.front();
        m_queue.pop();
        return data;
    }
};