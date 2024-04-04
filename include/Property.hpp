#pragma once
#include "PropertyBase.hpp"
#include <type_traits>

template <typename T>
concept Number = std::is_arithmetic_v<T>;

/**
 * @brief 创建一个属性值
 *
 * 属性值的读写是线程安全的
 *
 * 注意: 你不能在中断函数中读写属性值
 *
 * @tparam T 数值类型
 * @tparam access 访问级别
 */
template <Number T, Access access = Access::READ_WRITE>
struct Property : public PropertyAccess<access>
{
    Property(T value = 0)
    {
        safe_set(value);
    }

    /**
     * @brief 线程安全的读取
     *
     * @return T 属性值
     */
    virtual T safe_get() const
    {
        std::lock_guard lock(PropertyBase::mutex);
        return _value;
    }

    /**
     * @brief 线程安全的写入
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    virtual ErrorCode safe_set(T value)
    {
        std::lock_guard lock(PropertyBase::mutex);
        _value = value;
        return ErrorCode::S_OK;
    }

    /**
     * @brief 读取属性值
     *
     * 线程安全的读取
     *
     * @return T 属性值
     */
    operator T() const
    {
        return safe_get();
    }

    /**
     * @brief 写入属性值
     *
     * 线程安全的写入
     *
     * @tparam K
     * @param other
     * @return auto&
     */
    template <Number K>
    auto& operator=(const K other)
    {
        safe_set(other);
        return *this;
    }

    /**
     * @brief 获取属性值的地址
     *
     * 注意: 通过地址访问需要加锁
     *
     * @return T* 属性值地址
     */
    T* operator&()
    {
        return (T*)&_value;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        if (!extra.add(safe_get())) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(Extra& extra) override
    {
        T value;
        if (!extra.decode(value)) return ErrorCode::E_INVALID_ARG;

        safe_set(value);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add((uint16_t)sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T _value;
};
