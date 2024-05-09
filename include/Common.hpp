#pragma once
#include <cstddef>
#include <stdint.h>

#define IS_ENCRYPTED(cmd)        ((uint8_t)(cmd) & 0x80)
#define REMOVE_ENCRYPT_MARK(cmd) ((Command)((uint8_t)(cmd) & 0x7F))
#define ADD_ENCRYPT_MARK(cmd)    ((Command)((uint8_t)(cmd) | 0x80))

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

enum class Signal : uint8_t
{
    LOG      = 0, // CMD, log字符串
    POWER_UP = 1, // CMD, Application/Bootloader
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
     * CMD,S_OK,属性Id,属性值
     * CMD,E_NO_IMPLEMENT,属性Id
     * CMD,E_INVALID_ARG,属性Id
     * CMD,E_ID_NOT_EXIST,属性Id
     * CMD,E_NO_PERMISSION,属性Id
     * CMD,E_OUT_OF_BUFFER,属性Id
     */
    GET_PROPERTY,
    /**
     * @brief 写入属性值
     *
     * 请求: CMD,属性Id,属性值
     * 应答:
     * CMD,S_OK,属性Id
     * CMD,E_NO_IMPLEMENT,属性Id
     * CMD,E_INVALID_ARG,属性Id
     * CMD,E_ID_NOT_EXIST,属性Id
     * CMD,E_NO_PERMISSION,属性Id
     * CMD,E_READ_ONLY,属性Id
     * CMD,E_OVER_HIGH_LIMIT,属性Id
     * CMD,E_OVER_LOW_LIMIT,属性Id
     * CMD,E_ILLEGAL_STATE,属性Id
     */
    SET_PROPERTY,
    /**
     * @brief 获取属性值长度
     *
     * 请求: CMD,属性Id
     * 应答:
     * CMD,S_OK,属性Id,属性长度
     * CMD,E_INVALID_ARG,属性Id
     * CMD,E_ID_NOT_EXIST,属性Id
     * CMD,E_NO_PERMISSION,属性Id
     */
    GET_SIZE,
    /**
     * @brief 获取属性值描述
     *
     * 请求: CMD,属性Id
     * 应答:
     * CMD,S_OK,属性Id,属性描述
     * CMD,E_INVALID_ARG,属性Id
     * CMD,E_ID_NOT_EXIST,属性Id
     * CMD,E_NO_PERMISSION,属性Id
     */
    GET_DESC,
    /**
     * @brief 信号
     *
     * 请求: CMD,信号Id,信号参数
     * 应答: 无
     */
    SIGNAL,
};

/**
 * @brief 使用 CRC16-CCITT-false(ffff) 算法
 *
 */
using Chksum = uint16_t;

#pragma pack(1)

struct Request
{
    uint8_t address; // 从机地址
    Command cmd;     // 命令, MSB作为加密标记, 1=加密, 0=无加密
    uint8_t size;    // 附加参数长度
    Chksum  chksum;  // 帧头校验和
};

struct Response
{
    uint8_t   address; // 从机地址
    Command   cmd;     // 命令, MSB作为加密标记, 1=加密, 0=无加密
    uint8_t   size;    // 附加参数长度
    ErrorCode error;   // 错误码
    Chksum    chksum;  // 帧头校验和
};

#pragma pack()