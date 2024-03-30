#pragma once
#include <stdint.h>

enum class Command : uint8_t
{
    /**
     * @brief 回声; 发送什么就回应什么
     *
     * 请求: CMD,任意N字节数据(N>=0)
     * 应答:
     * CMD,S_OK,请求中的数据
     */
    ECHO = 0,
    /**
     * @brief 读取属性值
     *
     * 请求: CMD,属性Id
     * 应答:
     * CMD,S_OK,属性值
     * CMD,E_NO_PERMISSION
     * CMD,E_ID_NOT_EXIST
     * CMD,E_INVALID_ARG
     */
    GET_PROPERTY,
    /**
     * @brief 写入属性值
     *
     * 请求: CMD,属性Id,属性值
     * 应答:
     * CMD,S_OK
     * CMD,E_READ_ONLY
     * CMD,E_OVER_HIGH_LIMIT
     * CMD,E_OVER_LOW_LIMIT
     * CMD,E_ILLEGAL_STATE
     * CMD,E_ID_NOT_EXIST
     */
    SET_PROPERTY,
    /**
     * @brief 写入内存
     *
     * 请求: CMD,内存Id,地址偏移,N字节数据
     * 应答:
     * CMD,S_OK
     * CMD,E_READ_ONLY
     * CMD,E_OUT_OF_INDEX
     * CMD,E_ILLEGAL_STATE
     * CMD,E_ID_NOT_EXIST
     */
    SET_MEMORY,
    /**
     * @brief 读取内存
     *
     * 请求: CMD,内存Id,地址偏移,数据长度
     * 应答:
     * CMD,S_OK,内存Id,地址偏移,数据长度
     * CMD,E_NO_PERMISSION
     * CMD,E_OUT_OF_INDEX
     * CMD,E_ID_NOT_EXIST
     */
    GET_MEMORY,
};