#pragma once
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
     * @brief 读取内存
     *
     * 请求: CMD,内存Id,地址偏移,数据长度
     * 应答:
     * CMD,S_OK,内存Id,地址偏移,数据长度,N字节数据
     * CMD,E_NO_IMPLEMENT,内存Id,地址偏移,数据长度
     * CMD,E_INVALID_ARG,内存Id,地址偏移,数据长度
     * CMD,E_ID_NOT_EXIST,内存Id,地址偏移,数据长度
     * CMD,E_NO_PERMISSION,内存Id,地址偏移,数据长度
     * CMD,E_OUT_OF_INDEX,内存Id,地址偏移,数据长度
     * CMD,E_OUT_OF_BUFFER,内存Id,地址偏移,数据长度
     */
    GET_MEMORY,
    /**
     * @brief 写入内存
     *
     * 请求: CMD,内存Id,地址偏移,数据长度,N字节数据
     * 应答:
     * CMD,S_OK,内存Id,地址偏移,数据长度
     * CMD,E_NO_IMPLEMENT,内存Id,地址偏移,数据长度
     * CMD,E_INVALID_ARG,内存Id,地址偏移,数据长度
     * CMD,E_ID_NOT_EXIST,内存Id,地址偏移,数据长度
     * CMD,E_NO_PERMISSION,内存Id,地址偏移,数据长度
     * CMD,E_READ_ONLY,内存Id,地址偏移,数据长度
     * CMD,E_OUT_OF_INDEX,内存Id,地址偏移,数据长度
     * CMD,E_ILLEGAL_STATE,内存Id,地址偏移,数据长度
     */
    SET_MEMORY,
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
};

/**
 * @brief 使用 CRC16-CCITT-false(ffff) 算法
 *
 */
using Chksum = uint16_t;

struct Request
{
    uint8_t address; // 从机地址
    Command cmd;     // 命令
    uint8_t size;    // 附加参数长度
    Chksum  chksum;  // 帧头校验和
};

struct Response
{
    uint8_t   address; // 从机地址
    Command   cmd;     // 命令
    uint8_t   size;    // 附加参数长度
    ErrorCode error;   // 错误码
    Chksum    chksum;  // 帧头校验和
};