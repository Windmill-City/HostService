#pragma once
#include "PropertyBase.hpp"
#include <string.h>
#include <type_traits>

template <typename T, Access access = Access::READ_WRITE>
struct Struct : public PropertyAccess<access>
{
    T _value;

    Struct()
    {
        static_assert(std::is_class_v<T> && std::is_standard_layout_v<T>);
    }

    /* 隐式类型转换 */
    operator PropertyBase*()
    {
        return static_cast<PropertyBase*>(this);
    }

    /* 隐式类型转换 */
    operator T&()
    {
        return _value;
    }

    /* 地址运算符 */
    T* operator&()
    {
        return &_value;
    }

    /* 赋值运算符 */
    auto& operator=(const T& other)
    {
        _assign(other);
        return *this;
    }

    /* 赋值运算符 */
    auto& operator=(const Struct<T>& other)
    {
        if (this == &other) return *this;
        _assign(other._value);
        return *this;
    }

    /* getter */
    T& get()
    {
        return *this;
    }

    virtual ErrorCode _assign(const T& value)
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

        return _assign(*(T*)extra.data());
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add((uint16_t)sizeof(_value));
        return ErrorCode::S_OK;
    }
};
