#pragma once
#include "PropertyBase.hpp"
#include <type_traits>

template <typename T>
struct Property : public PropertyBase
{
    T _value;

    Property()
        : Property(0)
    {
    }

    Property(T value)
        : Property(value, Access::READ)
    {
    }

    Property(T value, Access access)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);

        this->access = access;
        _value       = value;
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

    virtual ErrorCode get(uint8_t** p_value, uint8_t& size) override
    {
        size     = sizeof(_value);
        *p_value = (uint8_t*)&_value;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(const uint8_t* p_value, const uint8_t size) override
    {
        if (size != sizeof(_value))
        {
            return ErrorCode::E_INVALID_ARG;
        }

        _assign(*(T*)p_value);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(uint16_t& size) override
    {
        size = sizeof(_value);
        return ErrorCode::S_OK;
    }
};
