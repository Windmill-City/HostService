#pragma once
#include <stdint.h>

enum class ErrorCode : uint8_t
{
    S_OK = 0,                // 执行成功
    E_NO_IMPLEMENT,          // 方法未实现
    E_INVALID_ARG,           // 参数有误
    E_NO_PERMISSION,         // 没有权限
    E_ID_NOT_EXIST,          // Id不存在
    E_READ_ONLY,             // 只读变量
    E_OUT_OF_INDEX,          // 内存访问越界
    E_OVER_HIGH_LIMIT,       // 超出上限
    E_OVER_LOW_LIMIT,        // 超出下限
    E_ILLEGAL_STATE,         // 非法状态
    E_OBJECT_SIZE_TOO_LARGE, // 对象大小超出帧长限制
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

struct PropertyBase
{
    Access            access;
    /**
     * @brief 设置属性值
     *
     * @param p_value [in]附加参数的指针
     * @param size [in]参数的长度
     * @return ErrorCode 错误码
     */
    virtual ErrorCode set(const uint8_t* p_value, const uint8_t size);
    /**
     * @brief 设置属性值(内存)
     *
     * @param offset [in]地址偏移
     * @param p_value [in]附加参数的指针
     * @param size [in]参数的长度
     * @return ErrorCode 错误码
     */
    virtual ErrorCode set_mem(const uint16_t offset, const uint8_t* p_value, const uint8_t datlen);
    /**
     * @brief 读取属性值
     *
     * @param p_value [out]数据的指针
     * @param size [out]数据的长度
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get(uint8_t** p_value, uint8_t& size);
    /**
     * @brief 读取属性值(内存)
     *
     * @param offset [in]地址偏移
     * @param p_value [out]数据的指针
     * @param size [in/out]数据的长度
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get_mem(const uint16_t offset, uint8_t** p_value, uint8_t& datlen);
    /**
     * @brief 获取属性长度
     *
     * @param size 属性长度
     * @return ErrorCode 错误码
     */
    virtual ErrorCode get_size(uint16_t& size);
};
