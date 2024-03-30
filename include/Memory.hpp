#pragma once
#include "PropertyBase.hpp"
#include <cstddef>

template <typename T, size_t len, Access access = Access::READ_WRITE>
struct Memory : public PropertyAccess<access>
{
    T             _value[len];

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

    virtual ErrorCode set_mem(const uint16_t offset, const uint8_t* p_value, const uint8_t datlen) override
    {
        // 检查是否超出内存区范围
        if (sizeof(_value) < offset + datlen) return ErrorCode::E_OUT_OF_INDEX;

        // 写入指定区段
        memcpy((uint8_t*)_value + offset, p_value, datlen);
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
        *p_value = (uint8_t*)_value + offset;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(uint16_t& size) override
    {
        size = sizeof(_value);
        return ErrorCode::S_OK;
    }
};