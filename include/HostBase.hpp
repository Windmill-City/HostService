#pragma once
#include <Common.hpp>
#include <FixedQueue.hpp>
#include <Property.hpp>

template <typename T>
struct Sync : public FixedQueue<sizeof(T), PopAction::PopOnPush>
{
    /**
     * @brief 验证是否是一个有效的帧头
     *
     * @return true 帧头有效
     * @return false 帧头无效
     */
    bool verify()
    {
        if (sizeof(T) != this->size()) return false;

        uint16_t chksum = CRC_START_CCITT_FFFF;
        for (size_t i = 0; i < this->size(); i++)
        {
            chksum = update_crc_ccitt(chksum, (*this)[i]);
        }
        return chksum == 0;
    }

    /**
     * @brief 获取帧头
     *
     * @return T 帧头
     */
    T get()
    {
        T item;
        for (size_t i = 0; i < sizeof(T); i++)
        {
            ((uint8_t*)&item)[i] = (*this)[i];
        }
        this->reset(); // 清空队列
        return item;
    }
};

using PropertyAddress = Property<uint8_t>;

struct HostBase
{
    // 从机地址
    const PropertyAddress& address;

    HostBase(const PropertyAddress& addr)
        : address(addr)
    {
    }

    /**
     * @brief 底层数据接收方法, 接收 1 字节数据
     *
     * @param rx 接收数据的变量
     * @return 是否接收成功
     */
    virtual bool rx(uint8_t& rx)                           = 0;
    /**
     * @brief 底层数据发送方法, 阻塞地发送任意长度字节
     *
     * @param _buf 要发送的数据
     * @param size 数据的长度
     */
    virtual void tx(const uint8_t* buf, const size_t size) = 0;
    /* 帧编码 */
    void         _encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint16_t size);
};
