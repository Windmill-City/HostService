#pragma once
#include "PropertyBase.hpp"
#include <type_traits>

template <typename T>
concept Number = std::is_arithmetic_v<T>;

template <Number T, Access access = Access::READ_WRITE>
struct Property : public PropertyAccess<access>
{
    T _value;

    Property(T value = 0)
    {
        _value = value;
    }

    /* 隐式类型转换 */
    operator T() const
    {
        return _value;
    }

    /* 赋值运算符 */
    template <typename K>
    auto& operator=(const K other)
    {
        _assign(other);
        return *this;
    }

    /* 赋值运算符 */
    template <typename K>
    auto& operator=(const Property<K>& other)
    {
        if (this == &other) return *this;
        _assign(other._value);
        return *this;
    }

    /* 地址运算符 */
    T* operator&()
    {
        return (T*)&_value;
    }

    /* 四则运算 */
    template <typename K>
    T operator+(const K rhs)
    {
        return this->_value + rhs;
    }

    template <typename K>
    T operator-(const K rhs)
    {
        return this->_value - rhs;
    }

    template <typename K>
    T operator*(const K rhs)
    {
        return this->_value * rhs;
    }

    template <typename K>
    T operator/(const K rhs)
    {
        return this->_value / rhs;
    }

    /* 二元运算符 */
    template <typename K>
    auto& operator+=(const K rhs)
    {
        _assign(this->_value + rhs);
        return *this;
    }

    template <typename K>
    auto& operator-=(const K rhs)
    {
        _assign(this->_value - rhs);
        return *this;
    }

    template <typename K>
    auto& operator*=(const K rhs)
    {
        _assign(this->_value * rhs);
        return *this;
    }

    template <typename K>
    auto& operator/=(const K rhs)
    {
        _assign(this->_value / rhs);
        return *this;
    }

    virtual ErrorCode _assign(T value)
    {
        _value = value;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        if (!extra.add(_value)) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(Extra& extra) override
    {
        if (extra.data_size() != sizeof(_value))
        {
            return ErrorCode::E_INVALID_ARG;
        }

        _assign(*(T*)extra.data());
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add((uint16_t)sizeof(_value));
        return ErrorCode::S_OK;
    }
};
