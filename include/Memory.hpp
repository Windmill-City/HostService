#pragma once
#include "PropertyBase.hpp"
#include <cstddef>

// 只允许标准布局类型
template <typename T>
concept MemoryData = std::is_standard_layout_v<T>;

#pragma pack(1)

struct MemoryAccess
{
    uint16_t offset; // 地址偏移
    uint16_t size;   // 数据长度

    bool     operator==(const MemoryAccess& access) const
    {
        return this->offset == access.offset && this->size == access.size;
    }
};

#pragma pack()

/**
 * @brief 内存区属性
 *
 * @note 内存区属性的读写分块进行, 需要额外的机制来保障数据的完整性
 *
 * @tparam T 标准布局类型
 * @tparam access 访问级别
 */
template <MemoryData T, Access _access = Access::READ_WRITE>
struct Memory : public PropertyAccess<_access>
{
    /**
     * @brief 获取内存区属性的字节长度
     *
     * @return uint16_t 字节长度
     */
    uint16_t size() const
    {
        return sizeof(_value);
    }

    /**
     * @brief 获取属性值首地址
     *
     * @return uint8_t*
     */
    uint8_t* operator&()
    {
        return (uint8_t*)&_value;
    }

    /**
     * @brief 按字节访问内存区内容
     *
     * @param idx 下标
     * @return T& 内容
     */
    auto& operator[](const size_t idx)
    {
        return _value[idx];
    }

    /**
     * @brief 获取属性值首地址
     *
     * @return uint8_t*
     */
    const uint8_t* operator&() const
    {
        return (uint8_t*)&_value;
    }

    /**
     * @brief 按字节访问内存区内容
     *
     * @param idx 下标
     * @return T& 内容
     */
    const auto& operator[](const size_t idx) const
    {
        return _value[idx];
    }

    virtual ErrorCode set(Extra& extra) override
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

    virtual ErrorCode get(Extra& extra) const override
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

    virtual ErrorCode get_size(Extra& extra) const override
    {
        extra.reset();
        extra.add<uint16_t>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T _value;
};