#pragma once
#include <FixedQueue.hpp>
#include <Property.hpp>

template <typename T>
struct Sync : public FixedQueue<sizeof(T) + sizeof(Checksum), PopAction::PopOnPush>
{
    /**
     * @brief 验证是否是一个有效的帧头
     *
     * @return true 帧头有效
     * @return false 帧头无效
     */
    bool verify() const
    {
        if (sizeof(T) + sizeof(Checksum) != this->size()) return false;

        Checksum chksum = CRC_START_CCITT_FFFF;
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

using PropertyNonce = Property<NonceType, Access::READ>;

struct PropertyKey : public Property<KeyType, Access::READ_WRITE_PROTECT>
{
    using parent = Property<KeyType, Access::READ_WRITE_PROTECT>;

    PropertyKey(KeyType& value)
        : parent(value)
    {
    }

    virtual ErrorCode get(Extra&, bool) const override final
    {
        // 不允许回读密钥
        return ErrorCode::E_NO_IMPLEMENT;
    }
};

struct SecretHolder
{
    // 密钥
    KeyType      key;
    // 随机数
    NonceType    nonce;

    /**
     * @brief 更新随机数
     *
     * @note 随机数应当使用随机数外设生成, 若没有对应的外设, 应当使用 MCU 的唯一 Id 作为随机数
     * @note 此随机数应当保证不同设备间是不同的
     * @note 在响应完毕一个加密的数据包之后, 此方法会被调用
     */
    virtual void update_nonce() = 0;
};

using PropertyAddress = Property<uint8_t>;

struct HostBase
{
    // 从机地址
    Address       address;
    // 密钥容器
    SecretHolder& secret;

    HostBase(Address address, SecretHolder& secret)
        : address(address)
        , secret(secret)
    {
    }

    void send(Command cmd, Extra& extra, bool encrypt = false, ErrorCode err = ErrorCode::S_OK);
    bool recv(Command& cmd, ErrorCode& err, Extra& extra);

  protected:
    /**
     * @brief 底层数据接收方法, 接收 1 字节数据
     *
     * @param rx 接收数据的变量
     * @return 是否接收成功
     */
    virtual bool rx(uint8_t& rx)                        = 0;
    /**
     * @brief 底层数据发送方法, 阻塞地发送任意长度字节
     *
     * @param buf 要发送的数据
     * @param size 数据的长度
     */
    virtual void tx(const void* buf, const size_t size) = 0;

  protected:
    void send(const Header& head, const void* extra, const Size size);
    bool sync(Header& head);

  protected:
    Sync<Header> _buf_head;
};
