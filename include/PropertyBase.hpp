#pragma once
#include <mutex>
#include <stdint.h>

enum class ErrorCode : uint8_t
{
    S_OK = 0,          // 执行成功
    E_NO_IMPLEMENT,    // 方法未实现
    E_INVALID_ARG,     // 参数有误
    E_ID_NOT_EXIST,    // Id不存在
    E_NO_PERMISSION,   // 没有权限
    E_OUT_OF_BUFFER,   // 超出帧长限制
    E_READ_ONLY,       // 只读变量
    E_OUT_OF_INDEX,    // 内存访问越界
    E_OVER_HIGH_LIMIT, // 超出上限
    E_OVER_LOW_LIMIT,  // 超出下限
    E_ILLEGAL_STATE,   // 非法状态
};

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

struct Extra;

struct PropertyBase
{
    /**
     * @brief 全局属性互斥量
     * 
     * 注意: 在读写属性值时必须先取得此互斥量
     */
    static std::mutex mutex;
    /**
     * @brief 设置属性值
     *
     * @param extra [in]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       set(Extra& extra);
    /**
     * @brief 设置属性值(内存)
     *
     * @param extra [in]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       set_mem(Extra& extra);
    /**
     * @brief 读取属性值
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       get(Extra& extra);
    /**
     * @brief 读取属性值(内存)
     *
     * @param extra [in/out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       get_mem(Extra& extra);
    /**
     * @brief 获取属性长度
     *
     * @param extra [out]附加参数
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       get_size(Extra& extra);
    /**
     * @brief 检查读取权限
     *
     * @param privileged 是否在特权模式
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       check_read(bool privileged) const  = 0;
    /**
     * @brief 检查写入权限
     *
     * @param privileged 是否在特权模式
     * @return ErrorCode 错误码
     */
    virtual ErrorCode       check_write(bool privileged) const = 0;
};

template <Access access>
struct PropertyAccess : public PropertyBase
{
    virtual ErrorCode check_read(bool privileged) const override
    {
        if (access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode check_write(bool privileged) const override
    {
        if (access == Access::READ) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && privileged) return ErrorCode::E_READ_ONLY;
        if (access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        if (access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
        return ErrorCode::S_OK;
    }
};
