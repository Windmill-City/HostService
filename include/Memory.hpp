#pragma once
#include "PropertyBase.hpp"

/**
 * @brief 内存区属性
 *
 * @note 内存区属性的读写分块进行, 需要额外的机制来保障数据的完整性
 *
 * @tparam T 标准布局类型
 * @tparam access 访问级别
 */
template <PropertyVal T, Access _access = Access::READ>
struct Memory : public PropertyAccess<_access>
{
    Memory(T& value)
        : _value(value)
    {
    }

    virtual ErrorCode set(Extra& extra, bool) override
    {
        MemoryAccess access;
        // 检查访问参数是否正确
        if (!extra.get(access) || access.size != extra.remain()) return ErrorCode::E_INVALID_ARG;
        // 检查是否超出内存区范围
        if (sizeof(_value) < access.offset + access.size) return ErrorCode::E_OUT_OF_INDEX;

        memcpy((uint8_t*)&_value + access.offset, extra.curr(), access.size);
        extra.reset();
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get(Extra& extra, bool) const override
    {
        MemoryAccess access;
        // 检查访问参数是否正确
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;
        // 检查是否超出内存区范围
        if (sizeof(_value) < access.offset + access.size)
        {
            return ErrorCode::E_OUT_OF_INDEX;
        }
        extra.reset();

        if (!extra.add((uint8_t*)&_value + access.offset, access.size)) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra, bool) const override
    {
        extra.reset();
        extra.add<Size>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T& _value;
};