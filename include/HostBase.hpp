#pragma once
#include <Common.hpp>

struct HostBase
{
    // 从机地址
    uint8_t         address;

    /**
     * @brief 轮询解析请求/响应
     *
     * @return true 成功解析一帧
     * @return false 没有接收到数据 或 请求/解析无效
     */
    virtual bool    poll()                                    = 0;
    /**
     * @brief 底层数据接收方法, 阻塞地接收 1 字节
     *
     * @return 接收到的 1 字节数据
     */
    virtual uint8_t rx()                                      = 0;
    /**
     * @brief 底层数据发送方法, 阻塞地发送任意长度字节
     *
     * @param _buf 要发送的数据
     * @param size 数据的长度
     */
    virtual void    tx(const uint8_t* buf, const size_t size) = 0;
    /* 帧编码 */
    void            _encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint8_t size);
};
