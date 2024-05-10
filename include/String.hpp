#pragma once
#include <array>
#include <stdint.h>

template <size_t _size>
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
        auto len = std::min(strlen(str), capacity());
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
    size_t capacity() const
    {
        return _size;
    }

  protected:
    std::array<char, _size + 1> _str;
};