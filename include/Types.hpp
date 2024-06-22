#pragma once
#include <array>
#include <stdint.h>
#include <type_traits>

#ifndef __packed
  #define __packed __attribute__((packed))
#endif

enum class ErrorCode : uint8_t
{
    S_OK = 0,          // 执行成功
    E_FAIL,            // 执行失败
    E_ALIGN,           // 未对齐的访问
    E_TIMEOUT,         // 执行超时
    E_BAD_BLOCK,       // 闪存写入失败
    E_READ_ONLY,       // 只读变量
    E_INVALID_ARG,     // 参数有误
    E_OUT_OF_INDEX,    // 内存访问越界
    E_NO_IMPLEMENT,    // 方法未实现
    E_ID_NOT_EXIST,    // Id不存在
    E_NO_PERMISSION,   // 没有权限
    E_OUT_OF_BUFFER,   // 超出帧长限制
    E_ILLEGAL_STATE,   // 非法状态
    E_OVER_LOW_LIMIT,  // 超出下限
    E_OVER_HIGH_LIMIT, // 超出上限
};

enum class LogLevel : uint8_t
{
    VERBOSE,
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

enum class RangeMode : uint8_t
{
    Hard,  // 超出范围的赋值 不会 生效
    Soft,  // 超出范围的赋值 会 生效
    Clamp, // 超出范围的赋值会被截断到范围以内
};

enum class RangeAccess : uint8_t
{
    Range = 0, // 当前范围
    Absolute   // 范围的绝对最大值
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
    READ_WRITE_PROTECT
};

enum class Command : uint8_t
{
    /**
     * @brief 回声; 发送什么就回应什么
     *
     * 请求: CMD,任意N字节数据(N>=0)
     * 应答:
     * CMD,S_OK,请求中的附加参数
     */
    ECHO = 0,
    /**
     * @brief 读取属性值
     *
     * 请求: CMD,属性Id
     * 应答:
     * CMD,S_OK,属性值
     */
    GET_PROPERTY,
    /**
     * @brief 写入属性值
     *
     * 请求: CMD,属性Id,属性值
     * 应答:
     * CMD,S_OK
     */
    SET_PROPERTY,
    /**
     * @brief 获取属性值长度
     *
     * 请求: CMD,属性Id
     * 应答:
     * CMD,S_OK,属性长度
     */
    GET_SIZE,
    /**
     * @brief Server 日志
     *
     * 请求: CMD,LogLevel,日志内容
     * 应答: 无
     */
    LOG
};

/**
 * @brief 校验和
 * @details 算法: CRC16-CCITT-False
 */
using Checksum   = uint16_t;
/**
 * @brief 属性值Id
 *
 */
using PropertyId = uint16_t;
/**
 * @brief 地址
 *
 */
using Address    = uint8_t;
/**
 * @brief 数据长度
 *
 */
using Size       = uint16_t;
/**
 * @brief CEC-MAC
 *
 */
using TagType    = std::array<uint8_t, 16>;
/**
 * @brief 随机数
 *
 */
using NonceType  = std::array<uint8_t, 12>;
/**
 * @brief 密钥
 *
 */
using KeyType    = std::array<uint8_t, 256 / 8>;

/**
 * @brief 属性值类型
 *
 * @tparam T 类型参数
 */
template <typename T>
concept PropertyVal = std::is_standard_layout_v<T> && !std::is_pointer_v<T>;

/**
 * @brief 数值类型
 *
 * @tparam T 类型参数
 */
template <typename T>
concept Number = std::is_arithmetic_v<T>;

/**
 * @brief 帧头
 *
 */
struct Header
{
    Address   address; // 从机地址
    Command   cmd;     // 命令, MSB作为加密标记, 1=加密, 0=无加密
    Size      size;    // 附加参数长度
    ErrorCode error;   // 错误码
} __packed;

/**
 * @brief 内存属性值访问参数
 *
 */
struct MemoryAccess
{
    Size offset; // 地址偏移
    Size size;   // 数据长度

    bool operator==(const MemoryAccess& access) const
    {
        return this->offset == access.offset && this->size == access.size;
    }
} __packed;
