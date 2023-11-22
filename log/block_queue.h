//
// Created by 丁长飞 on 2023/11/22.
//

/*
 * 循环数组实现的阻塞队列，m_back = (m_back + 1) % m_max_size;
 * 线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
*/

#ifndef WEBSERVER_BLOCK_QUEUE_H
#define WEBSERVER_BLOCK_QUEUE_H

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/time.h>

#include "../lock/locker.h"

using namespace std;

template <class T>
class block_queue
{
public:
    //构造函数
    block_queue(int max_size = 1000)
    {
        //如果max_size小于等于0,则抛出异常
        if (max_size <= 0)
        {
            exit(-1);
        }

        //设置队列最大长度
        m_max_size = max_size;
        //初始化队列
        m_array = new T[max_size];
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    //清空队列
    void clear()
    {
        //加锁
        m_mutex.lock();
        //将队列长度置为0
        m_size = 0;
        //将队列头尾指针置为-1
        m_front = -1;
        m_back = -1;
        //释放锁
        m_mutex.unlock();
    }

    //析构函数
    ~block_queue()
    {
        //加锁
        m_mutex.lock();
        //如果队列不为空,则将队列释放
        if (m_array != NULL)
            delete [] m_array;

        //释放锁
        m_mutex.unlock();
    }

    //判断队列是否满了
    bool full()
    {
        //加锁
        m_mutex.lock();
        //如果队列长度等于最大长度,则返回true
        if (m_size >= m_max_size)
        {

            //释放锁
            m_mutex.unlock();
            return true;
        }
        //释放锁
        m_mutex.unlock();
        return false;
    }

    //判断队列是否为空
    bool empty()
    {
        //加锁
        m_mutex.lock();
        //如果队列长度为0,则返回true
        if (0 == m_size)
        {
            //释放锁
            m_mutex.unlock();
            return true;
        }
        //释放锁
        m_mutex.unlock();
        return false;
    }

    //返回队首元素
    bool front(T &value)
    {
        //加锁
        m_mutex.lock();
        //如果队列长度为0,则返回false
        if (0 == m_size)
        {
            //释放锁
            m_mutex.unlock();
            return false;
        }
        //获取队列头元素
        value = m_array[m_front];
        //释放锁
        m_mutex.unlock();
        return true;
    }

    //返回队尾元素
    bool back(T &value)
    {
        //加锁
        m_mutex.lock();
        //如果队列长度为0,则返回false
        if (0 == m_size)
        {
            //释放锁
            m_mutex.unlock();
            return false;
        }
        //获取队列尾元素
        value = m_array[m_back];
        //释放锁
        m_mutex.unlock();
        return true;
    }

    //返回队列长度
    int size()
    {
        int tmp = 0;
        //加锁
        m_mutex.lock();
        //获取队列长度
        tmp = m_size;
        //释放锁
        m_mutex.unlock();
        return tmp;
    }

    //返回队列最大长度
    int max_size()
    {
        int tmp = 0;
        //加锁
        m_mutex.lock();
        //获取队列最大长度
        tmp = m_max_size;
        //释放锁
        m_mutex.unlock();
        return tmp;
    }

    //往队列添加元素，需要将所有使用队列的线程先唤醒
    //当有元素push进队列,相当于生产者生产了一个元素
    //若当前没有线程等待条件变量,则唤醒无意义
    bool push(const T &item)
    {
        //加锁
        m_mutex.lock();
        //如果队列长度等于最大长度,则返回false
        if (m_size >= m_max_size)
        {
            //唤醒所有等待的线程
            m_cond.broadcast();
            //释放锁
            m_mutex.unlock();
            return false;
        }

        //获取队列尾指针
        m_back = (m_back + 1) % m_max_size;
        //将元素添加到队列
        m_array[m_back] = item;

        //队列长度加1
        m_size++;
        //唤醒所有等待的线程
        m_cond.broadcast();
        //释放锁
        m_mutex.unlock();
        return true;
    }

    //pop时,如果当前队列没有元素,将会等待条件变量
    bool pop(T &item)
    {
        //加锁
        m_mutex.lock();
        //如果队列长度为0,则等待条件变量
        while (m_size <= 0)
        {
            //等待条件变量
            if (!m_cond.wait(m_mutex.get()))
            {
                //释放锁
                m_mutex.unlock();
                return false;
            }
        }

        //获取队列头指针
        m_front = (m_front + 1) % m_max_size;
        //获取队列头元素
        item = m_array[m_front];
        //队列长度减1
        m_size--;
        //释放锁
        m_mutex.unlock();
        return true;
    }

    //增加了超时处理
    bool pop(T &item, int ms_timeout)
    {
        //设置超时时间
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        //加锁
        m_mutex.lock();
        //如果队列长度为0,则等待超时时间
        if (m_size <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timewait(m_mutex.get(), t))
            {
                //释放锁
                m_mutex.unlock();
                return false;
            }
        }

        //如果队列长度为0,则等待超时时间
        if (m_size <= 0)
        {
            //释放锁
            m_mutex.unlock();
            return false;
        }

        //获取队列头指针
        m_front = (m_front + 1) % m_max_size;
        //获取队列头元素
        item = m_array[m_front];
        //队列长度减1
        m_size--;
        //释放锁
        m_mutex.unlock();
        return true;
    }

private:
    locker m_mutex;
    cond m_cond;

    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};

#endif //WEBSERVER_BLOCK_QUEUE_H
