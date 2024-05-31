#pragma once
#include <Common.hpp>

/**
 * @brief 互斥锁
 *
 */
struct Mutex
{
    /**
     * @brief 获得锁
     *
     */
    virtual void acquire()
    {
    }

    /**
     * @brief 释放锁
     *
     */
    virtual void release()
    {
    }
};

/**
 * @brief Mutex 的 RAII 包装类
 *
 */
struct LockGuard
{
    LockGuard(Mutex& mutex)
        : _mutex(mutex)
    {
        _mutex.acquire();
    }

    ~LockGuard()
    {
        _mutex.release();
    }

  protected:
    Mutex& _mutex;
};
