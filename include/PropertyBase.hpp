#pragma once
#include <Common.hpp>
#include <Extra.hpp>
#include <Mutex.hpp>
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
    READ_WRITE_PROTECT
};

/**
 * @brief 属性值Id
 *
 */
using PropertyId = uint16_t;

struct PropertyBase
{
    static Mutex&     MutexGlobal;
    /**
     * @brief 设置属性值
     *
     * @param extra [in]附加参数
     * @param privileged [in]特权模式
     * @return ErrorCode 错误码
     */
    virtual ErrorCode set(Extra& extra, bool privileged);
    /**
     * @brief 读取属性值
     *
     * @param extra [in/out]附加参数
     * @param privileged [in]特权模式
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get(Extra& extra, bool privileged) const;
    /**
     * @brief 获取属性长度
     *
     * @param extra [out]附加参数
     * @param privileged [in]特权模式
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get_size(Extra& extra, bool privileged) const;
    /**
     * @brief 检查读取参数
     *
     * @param privileged 特权模式?
     * @return ErrorCode 错误码
     */
    virtual ErrorCode check_read(const bool privileged) const  = 0;
    /**
     * @brief 检查写入参数
     *
     * @param privileged 特权模式?
     * @return ErrorCode 错误码
     */
    virtual ErrorCode check_write(const bool privileged) const = 0;
};

template <Access access>
struct PropertyAccess : public PropertyBase
{
    virtual ErrorCode check_read(const bool privileged) const override
    {
        if (access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode check_write(const bool privileged) const override
    {
        if (access == Access::READ) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && privileged) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }
};
