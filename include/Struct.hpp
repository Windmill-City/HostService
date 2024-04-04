#pragma once
#include "PropertyBase.hpp"
#include <algorithm>
#include <array>
#include <string.h>
#include <type_traits>

template <typename T>
concept _Struct = std::is_class_v<T> && std::is_standard_layout_v<T>;

template <size_t _len>
struct String
{
    String(const char* str = "")
    {
        set(str);
    }

    /**
     * @brief 设置字符串值
     *
     * @param str 要设置的字符串
     * @return 自身的引用
     */
    auto& operator=(const char* str)
    {
        set(str);
        return *this;
    }

    /**
     * @brief 隐式转换为字符串字面量
     *
     * @return const char* 字符串字面量
     */
    operator const char*() const
    {
        return _str.data();
    }

    /**
     * @brief 设置字符串值
     *
     * @param str 要设置的字符串
     * @return 自身的引用
     */
    auto& set(const char* str)
    {
        auto len = std::min(strlen(str), max_size());
        memcpy(_str.data(), str, len);
        _str[len] = '\0';
        return *this;
    }

    /**
     * @brief 返回字符串长度
     *
     * @return size_t 字符串长度
     */
    size_t size() const
    {
        return strlen(_str);
    }

    /**
     * @brief 返回字符串最大长度, 不包括\0
     *
     * @return size_t 字符串最大长度
     */
    size_t max_size() const
    {
        static_assert(_len >= 1);
        return _len - 1;
    }

  protected:
    std::array<char, _len> _str;
};

/**
 * @brief 创建结构体属性值
 *
 * 属性值的读写是线程安全的
 *
 * 注意: 你不能在中断函数中读写属性值
 *
 * @tparam T 结构体类型
 * @tparam access 访问级别
 */
template <_Struct T, Access access = Access::READ_WRITE>
struct Struct : public PropertyAccess<access>
{
    /**
     * @brief 线程安全的读取
     *
     * @return T 读取的值
     */
    virtual T safe_get() const
    {
        return _value;
    }

    /**
     * @brief 线程安全的写入
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    virtual ErrorCode safe_set(const T value)
    {
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
     * @brief 获取属性的引用
     *
     * 注意: 通过引用访问需要加锁
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
     * 注意: 通过地址访问需要加锁
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
     * 线程安全的写入
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
