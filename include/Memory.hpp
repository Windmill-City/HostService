#pragma once
#include "PropertyBase.hpp"
#include <cstddef>

template <typename T>
concept _Memory = std::is_standard_layout_v<T>;

template <_Memory T, size_t _len, Access access = Access::READ_WRITE>
struct Memory : public PropertyAccess<access>
{
    T _value[_len];

    Memory()
    {
        // 确保所有内容都能访问到
        static_assert(sizeof(_value) < UINT16_MAX + UINT8_MAX);
    }

    /**
     * @brief 获取数组长度
     *
     * @return size_t 数组长度
     */
    size_t len()
    {
        return _len;
    }

    /**
     * @brief 获取内存区字节长度
     *
     * @return uint16_t 字节长度
     */
    uint16_t size()
    {
        return sizeof(_value);
    }

    /* 隐式类型转换 */
    operator PropertyBase*()
    {
        return static_cast<PropertyBase*>(this);
    }

    /* 地址运算符 */
    T* operator&()
    {
        return (T*)_value;
    }

    /* 数组运算符 */
    template <typename K>
    T& operator[](K idx)
    {
        return _value[idx];
    }

    virtual ErrorCode set_mem(Extra& extra) override
    {
        uint16_t offset = extra.offset();
        uint8_t  datlen = extra.datlen();
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen) return ErrorCode::E_OUT_OF_INDEX;

        memcpy((uint8_t*)_value + offset, extra.data(), datlen);
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

        if (!extra.add((uint8_t*)&_value + offset, datlen)) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add((uint16_t)sizeof(_value));
        return ErrorCode::S_OK;
    }
};