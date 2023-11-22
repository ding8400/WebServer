//
// Created by 丁长飞 on 2023/11/22.
//

#ifndef WEBSERVER_LOCKER_H
#define WEBSERVER_LOCKER_H


#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 声明一个信号量类
class sem
{
public:
    // 构造函数，初始化信号量
    sem()
    {
        // 初始化信号量，初始值为0
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            // 如果初始化失败，抛出异常
            throw std::exception();
        }
    }

    // 构造函数，初始化信号量，初始值为num
    sem(int num)
    {
        // 初始化信号量，初始值为num
        if (sem_init(&m_sem, 0, num) != 0)
        {
            // 如果初始化失败，抛出异常
            throw std::exception();
        }
    }

    // 构造函数，销毁信号量
    ~sem()
    {
        // 销毁信号量
        sem_destroy(&m_sem);
    }

    // 等待信号量，成功返回true，失败返回false
    bool wait()
    {
        // 等待信号量
        return sem_wait(&m_sem) == 0;
    }

    // 释放信号量，成功返回true，失败返回false
    bool post()
    {
        // 释放信号量
        return sem_post(&m_sem) == 0;
    }

private:
    // 声明一个信号量
    sem_t m_sem;
};


// 声明一个互斥锁类
class locker
{
public:
    // 构造函数，初始化锁
    locker()
    {
        // 初始化锁
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            // 如果初始化失败，抛出异常
            throw std::exception();
        }
    }
    // 构造函数，销毁锁
    ~locker()
    {
        // 销毁锁
        pthread_mutex_destroy(&m_mutex);
    }
    // 加锁，成功返回true，失败返回false
    bool lock()
    {
        // 加锁
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    // 解锁，成功返回true，失败返回false
    bool unlock()
    {
        // 解锁
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    // 获取锁
    pthread_mutex_t *get()
    {
        // 返回锁
        return &m_mutex;
    }

private:
    // 声明一个锁
    pthread_mutex_t m_mutex;
};


// 声明一个条件变量类
class cond
{
public:
    // 构造函数，初始化条件变量
    cond()
    {
        // 初始化条件变量
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            // 如果初始化失败，抛出异常
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }

    // 构造函数，销毁条件变量
    ~cond()
    {
        // 销毁条件变量
        pthread_cond_destroy(&m_cond);
    }

    // 等待条件变量，成功返回true，失败返回false
    bool wait(pthread_mutex_t *m_mutex)
    {
        // 等待条件变量
        int ret = 0;
        // 加锁
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        // 解锁
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    // 等待条件变量，成功返回true，失败返回false
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        // 等待条件变量
        int ret = 0;
        // 加锁
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        // 解锁
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    // 信号条件变量，成功返回true，失败返回false
    bool signal()
    {
        // 信号条件变量
        return pthread_cond_signal(&m_cond) == 0;
    }

    // 广播条件变量，成功返回true，失败返回false
    bool broadcast()
    {
        // 广播条件变量
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    // 声明一个锁
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif //WEBSERVER_LOCKER_H
