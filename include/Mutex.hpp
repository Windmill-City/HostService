#pragma once
#include <Common.hpp>

/**
 * @brief 互斥锁
 *
 */
struct Mutex
{
    virtual void lock()   = 0;
    virtual void unlock() = 0;
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
        _mutex.lock();
    }

    ~LockGuard()
    {
        _mutex.unlock();
    }

  protected:
    Mutex& _mutex;
};
