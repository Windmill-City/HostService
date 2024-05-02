#pragma once
#include <Common.hpp>
#include <Extra.hpp>
#include <stdint.h>

enum class Access : uint8_t
{
    /**
     *         |READ|READ_WRITE|WRITE_PROTECT|READ_PROTECT|READ_WRITE_PROTECT|
     * 普通模式|   r|         rw|           r |            |                  |
     * 特权模式|   r|         rw|           rw|           r|                rw|
     *
     */
    READ = 0,
    READ_WRITE,
    WRITE_PROTECT,
    READ_PROTECT,
    READ_WRITE_PROTECT,
};

/**
 * @brief 属性值Id
 *
 */
using PropertyId = uint16_t;

struct PropertyBase
{
    static AES        Key;
    /**
     * @brief 属性值唯一名
     *
     */
    const char*       unique;
    /**
     * @brief 设置属性值
     *
     * @param extra [in]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode set(Extra& extra);
    /**
     * @brief 设置属性值(内存)
     *
     * @param extra [in]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode set_mem(Extra& extra);
    /**
     * @brief 读取属性值
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get(Extra& extra);
    /**
     * @brief 读取属性值(内存)
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get_mem(Extra& extra);
    /**
     * @brief 获取属性长度
     *
     * @param extra [out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get_size(Extra& extra);
    /**
     * @brief 检查读取参数
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode check_read(Extra& extra) const  = 0;
    /**
     * @brief 检查写入参数
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode check_write(Extra& extra) const = 0;
};

template <Access access>
struct PropertyAccess : public PropertyBase
{
    virtual ErrorCode check_read(Extra& extra) const override
    {
        if (access == Access::READ_PROTECT && !extra.decrypt(Key)) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !extra.decrypt(Key)) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode check_write(Extra& extra) const override
    {
        if (access == Access::READ) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && extra.decrypt(Key)) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && !extra.decrypt(Key)) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::WRITE_PROTECT && !extra.decrypt(Key)) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !extra.decrypt(Key)) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }
};
