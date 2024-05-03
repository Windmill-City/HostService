#pragma once
#include "PropertyBase.hpp"
#include <cstddef>

// 只允许标准布局类型
template <typename T>
concept MemoryData = std::is_standard_layout_v<T>;

struct MemoryAccess
{
    uint16_t offset;
    uint8_t  size;
};

/**
 * @brief 在内存中创建定长数组
 *
 * 注意: 数组的读写分块进行, 需要额外的机制来保障数据的完整性
 *
 * @tparam T 标准布局类型
 * @tparam _len 数组长度
 * @tparam access 访问级别
 */
template <MemoryData T, size_t _len, Access access = Access::READ_WRITE>
struct Memory : public PropertyAccess<access>
{
    Memory()
    {
        // 确保所有内容都能访问到
        const frame_size_max = UINT8_MAX;
        const offset_max     = UINT16_MAX;
        static_assert(sizeof(_value) < offset_max + frame_size_max);
    }

    /**
     * @brief 获取数组长度
     *
     * @return size_t 数组长度
     */
    size_t len() const
    {
        return _len;
    }

    /**
     * @brief 获取数组字节长度
     *
     * @return uint16_t 字节长度
     */
    uint16_t size() const
    {
        return sizeof(_value);
    }

    /**
     * @brief 获取数组首地址
     *
     * @return T*
     */
    T* operator&()
    {
        return _value;
    }

    /**
     * @brief 以数组下标的方式访问数组内容
     *
     * @param idx 下标
     * @return T& 内容
     */
    T& operator[](size_t idx)
    {
        return _value[idx];
    }

    virtual ErrorCode set_mem(Extra& extra) override
    {
        MemoryAccess access;
        // 检查访问参数是否正确
        if (!extra.get(access) || access.size != extra.remain()) return ErrorCode::E_INVALID_ARG;
        // 检查是否超出内存区范围
        if (sizeof(_value) < access.offset + access.size) return ErrorCode::E_OUT_OF_INDEX;

        memcpy((uint8_t*)_value + access.offset, extra.data(), access.size);
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_mem(Extra& extra) override
    {
        MemoryAccess access;
        // 检查访问参数是否正确
        if (!extra.get(access) || access.size != extra.remain()) return ErrorCode::E_INVALID_ARG;
        // 检查是否超出内存区范围
        if (sizeof(_value) < access.offset + access.size)
        {
            return ErrorCode::E_OUT_OF_INDEX;
        }

        if (!extra.add((uint8_t*)&_value + access.offset, access.size)) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.add<uint16_t>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T _value[_len];
};