#pragma once
#include "PropertyBase.hpp"
#include <string.h>
#include <type_traits>

template <typename T>
struct Struct : public PropertyBase
{
    T _value;

    Struct()
        : Struct(Access::READ)
    {
    }

    Struct(Access access)
    {
        static_assert(std::is_class_v<T> && std::is_standard_layout_v<T>);
        this->access = access;
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

    virtual ErrorCode get(uint8_t** p_value, uint8_t& size) override
    {
        if (sizeof(_value) > UINT8_MAX)
        {
            size = 0;
            return ErrorCode::E_OBJECT_SIZE_TOO_LARGE;
        }

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

        _value = *(T*)p_value;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set_mem(const uint16_t offset, const uint8_t* p_value, const uint8_t datlen) override
    {
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen) return ErrorCode::E_OUT_OF_INDEX;

        // 写入指定区段
        memcpy((uint8_t*)&_value + offset, p_value, datlen);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_mem(const uint16_t offset, uint8_t** p_value, uint8_t& datlen)
    {
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen)
        {
            datlen = 0;
            return ErrorCode::E_OUT_OF_INDEX;
        }

        // 返回数据区段
        *p_value = (uint8_t*)&_value + offset;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(uint16_t& size) override
    {
        size = sizeof(_value);
        return ErrorCode::S_OK;
    }
};
