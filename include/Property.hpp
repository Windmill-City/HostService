#pragma once
#include "PropertyBase.hpp"
#include <type_traits>

template <typename T, Access access = Access::READ_WRITE>
struct Property : public PropertyAccess<access>
{
    T _value;

    Property(T value = 0)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);

        _value = value;
    }

    /* 隐式类型转换 */
    operator T() const
    {
        return _value;
    }

    /* 隐式类型转换 */
    operator PropertyBase*()
    {
        return static_cast<PropertyBase*>(this);
    }

    /* 赋值运算符 */
    auto& operator=(const T other)
    {
        _assign(other);
        return *this;
    }

    /* 赋值运算符 */
    auto& operator=(const Property<T>& other)
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
    T operator+(const T rhs)
    {
        return this->_value + rhs;
    }

    T operator-(const T rhs)
    {
        return this->_value - rhs;
    }

    T operator*(const T rhs)
    {
        return this->_value * rhs;
    }

    T operator/(const T rhs)
    {
        return this->_value / rhs;
    }

    /* 二元运算符 */
    auto& operator+=(const T rhs)
    {
        _assign(this->_value + rhs);
        return *this;
    }

    auto& operator-=(const T rhs)
    {
        _assign(this->_value - rhs);
        return *this;
    }

    auto& operator*=(const T rhs)
    {
        _assign(this->_value * rhs);
        return *this;
    }

    auto& operator/=(const T rhs)
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
        if (!extra.add(_value)) return ErrorCode::E_OBJECT_SIZE_TOO_LARGE;
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
