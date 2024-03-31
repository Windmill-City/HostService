#pragma once
#include "PropertyBase.hpp"
#include <cstddef>

template <typename T, uint16_t _len, Access access = Access::READ_WRITE>
struct Memory : public PropertyAccess<access>
{
    T _value[_len];

    Memory()
    {
        // 确保所有内容都能访问到
        static_assert(sizeof(_value) < UINT16_MAX + UINT8_MAX);
    }

    /**
     * @brief 获取内存区长度
     *
     * @return uint16_t 内存区长度
     */
    uint16_t len()
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

    /**
     * @brief 返回指向基类的指针
     *
     * 由于隐式转换到 PropertyBase* 会引起operator[]运算符的调用歧义
     * 这里引入额外的方法来获取指向基类的指针
     *
     * @return PropertyBase* 指向基类的指针
     */
    PropertyBase* base()
    {
        return static_cast<PropertyBase*>(this);
    }

    /* 显式类型转换 */
    explicit operator PropertyBase*()
    {
        return static_cast<PropertyBase*>(this);
    }

    /* 地址运算符 */
    T* operator&()
    {
        return (T*)_value;
    }

    /* 数组运算符 */
    T& operator[](std::size_t idx)
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