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
    auto& operator=(const T other)
    {
        _value = other;
        return *this;
    }

    /* 赋值运算符 */
    auto& operator=(const Struct<T>& other)
    {
        if (this == &other) return *this;
        _value = other._value;
        return *this;
    }

    /* getter */
    T& get()
    {
        return *this;
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

        _value = *(T*)extra.data();
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set_mem(Extra& extra) override
    {
        uint16_t offset = extra.offset();
        uint8_t  datlen = extra.datlen();
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen) return ErrorCode::E_OUT_OF_INDEX;

        memcpy((uint8_t*)&_value + offset, extra.data(), datlen);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_mem(Extra& extra) override
    {
        uint16_t offset = extra.offset();
        uint8_t  datlen = extra.datlen();
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen)
        {
            return ErrorCode::E_OUT_OF_INDEX;
        }

        if (!extra.add((uint8_t*)&_value + offset, datlen)) return ErrorCode::E_OBJECT_SIZE_TOO_LARGE;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add((uint16_t)sizeof(_value));
        return ErrorCode::S_OK;
    }
};
