#pragma once
#include "PropertyBase.hpp"
#include <algorithm>
#include <string.h>
#include <type_traits>

template <typename T>
concept _Struct = std::is_class_v<T> && std::is_standard_layout_v<T>;

/**
 * @brief 结构体属性值模板
 *
 * @tparam T 结构体类型
 * @tparam access 访问级别
 */
template <_Struct T, Access access = Access::READ_WRITE>
struct Struct : public PropertyAccess<access>
{
    Struct(T value = {0})
    {
        safe_set(value);
    }

    /**
     * @brief 设置属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    virtual T safe_get() const
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        return _value;
    }

    /**
     * @brief 读取属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return T 属性值
     */
    virtual ErrorCode safe_set(const T value)
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        _value = value;
        return ErrorCode::S_OK;
    }

    /**
     * @brief 读取属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return T 属性值
     */
    operator T() const
    {
        return safe_get();
    }

    /**
     * @brief 获取属性的引用
     *
     * @note 此方法非线程安全
     *
     * @return T& 属性值引用
     */
    T& ref()
    {
        return _value;
    }

    /**
     * @brief 获取属性的地址
     *
     * @note 此方法非线程安全
     *
     * @return T* 属性地址
     */
    T* operator&()
    {
        return &_value;
    }

    /**
     * @brief 写入属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @param other 要写的值
     * @return auto& 自身的引用
     */
    auto& operator=(const T other)
    {
        safe_set(other);
        return *this;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        if (!extra.add(safe_get())) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(Extra& extra) override
    {
        T value;
        if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;

        safe_set(value);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add<uint16_t>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T _value;
};
