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

using PropertyNonce = Property<NonceType, Access::READ>;

struct PropertyKey : public Property<KeyType, Access::READ_WRITE_PROTECT>
{
    using parent = Property<KeyType, Access::READ_WRITE_PROTECT>;

    PropertyKey(KeyType value = {0})
        : parent(value)
    {
    }

    /**
     * @brief 读取属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return 密钥
     */
    operator KeyType() const
    {
        return safe_get();
    }

    /**
     * @brief 获取属性的地址
     *
     * @note 此方法非线程安全
     *
     * @return T* 属性地址
     */
    uint8_t* operator&()
    {
        return this->_value.data();
    }

    /**
     * @brief 写入属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @param other 要写的值
     * @return auto& 自身的引用
     */
    auto& operator=(const KeyType other)
    {
        safe_set(other);
        return *this;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        // 不允许回读密钥
        return ErrorCode::E_NO_IMPLEMENT;
    }
};

struct SecretHolder
{
    // 加密通信随机数, 用来防止重放攻击
    PropertyNonce nonce;
    // 加密通信密钥
    PropertyKey   key;

    /**
     * @brief 更新随机数
     *
     * @note 随机数应当使用随机数外设生成, 若没有对应的外设, 应当使用MCU的唯一Id作为随机数
     * @note 此随机数应当保证不同设备间是不同的
     * @note 在响应完毕一个加密的数据包之后, 此方法会被调用
     */
    virtual void  update_nonce()
    {
    }
};

using PropertyAddress = Property<uint8_t>;

struct HostBase
{
    // 从机地址
    const PropertyAddress& address;
    // 密钥容器
    SecretHolder&          secret;

    HostBase(const PropertyAddress& addr, SecretHolder& secret)
        : address(addr)
        , secret(secret)
    {
    }

    void send_request(const Command cmd, Extra& extra, const bool encrypt = false);
    bool recv_request(Command& cmd, Extra& extra);

    void send_response(const Command cmd, const ErrorCode err, Extra& extra);
    bool recv_response(const Command cmd, ErrorCode& err, Extra& extra);

    bool recv_ack();

  protected:
    /**
     * @brief 底层数据接收方法, 接收 1 字节数据
     *
     * @param rx 接收数据的变量
     * @return 是否接收成功
     */
    virtual bool rx(uint8_t& rx)                                = 0;
    /**
     * @brief 底层数据发送方法, 阻塞地发送任意长度字节
     *
     * @param _buf 要发送的数据
     * @param size 数据的长度
     */
    virtual void tx(const uint8_t* buf, const size_t size)      = 0;
    /**
     * @brief 日志输出接口
     *
     * @param log 日志信息
     * @param size 日志字节长度
     */
    virtual void log_output(const char* log, const size_t size) = 0;

  protected:
    void _encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint16_t size);

  protected:
    Sync<Response> _buf_rep;
    Sync<Request>  _buf_req;
};
