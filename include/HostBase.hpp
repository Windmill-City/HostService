#pragma once
#include <Command.hpp>
#include <map>
#include <PropertyBase.hpp>

/**
 * @brief 使用 CRC16-CCITT-false(ffff)格式
 *
 */
using Chksum = uint16_t;

#pragma pack(1)

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

#pragma pack()

using PropertyHolder = std::map<uint16_t, PropertyBase*>;

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
    /* 帧编码/解码 */
    void            _encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint8_t size);
    bool            _decode_head(uint8_t* head, uint8_t h_size);
};
