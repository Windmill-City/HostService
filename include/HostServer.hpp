#pragma once
#include <algorithm>
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <frozen/string.h>
#include <HostBase.hpp>

template <size_t _size>
using PropertyMap   = std::array<std::pair<frozen::string, PropertyBase*>, _size>;
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

struct PropertyHolderBase
{
    /**
     * @brief 根据属性id获取属性值
     *
     * @param id 属性id
     * @return PropertyBase* 指向属性值的指针
     */
    virtual PropertyBase*  get(PropertyId id) const      = 0;
    /**
     * @brief 根据属性id获取属性值描述
     *
     * @param id 属性id
     * @return 属性值描述
     */
    virtual frozen::string get_desc(PropertyId id) const = 0;
    /**
     * @brief 获取属性值个数
     *
     * @return size_t 属性值个数
     */
    virtual size_t         size() const                  = 0;
};

struct PropertySymbols : public PropertyAccess<Access::READ>
{
    virtual ErrorCode get(Extra& extra) override
    {
        PropertyId id;
        if (!extra.get(id)) return ErrorCode::E_INVALID_ARG;
        if (id >= _holder->size()) return ErrorCode::E_OUT_OF_INDEX;

        frozen::string desc = _holder->get_desc(id);
        extra.reset();
        extra.add(desc.data(), desc.size());
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        if (_holder->size() > UINT16_MAX) return ErrorCode::E_OUT_OF_INDEX;
        extra.reset();
        extra.add<uint16_t>(_holder->size());
        return ErrorCode::S_OK;
    }

  protected:
    PropertyHolderBase* _holder = nullptr;
};

template <size_t _size>
struct PropertyHolder : public PropertyHolderBase
{
    using Map = PropertyMap<_size>;
    const Map& map;

    PropertyHolder(const Map& map)
        : map(map)
    {
    }

    PropertyHolder(const Map& map, PropertySymbols& ids)
        : map(map)
    {
        ids._holder = this;
    }

    virtual PropertyBase* get(PropertyId id) const override
    {
        if (id >= map.size()) return nullptr;
        return map[id].second;
    }

    virtual frozen::string get_desc(PropertyId id) const override
    {
        if (id >= map.size()) return "";
        return map[id].first;
    }

    virtual size_t size() const override
    {
        return map.size();
    }
};

struct HostServer : public HostBase
{
    // 密钥容器
    SecretHolder& secret;
    // Signal 发送缓冲区
    Extra         sig_extra;

    HostServer(const PropertyAddress& addr, const PropertyHolderBase& holder, SecretHolder& secret)
        : HostBase(addr)
        , secret(secret)
        , _holder(holder)
    {
    }

    bool poll();
    bool recv_request(Command& cmd, Extra& extra);
    void send_response(const Command cmd, const ErrorCode err, Extra& extra);
    void send_signal(Extra& extra);

  protected:
    PropertyBase* _acquire_and_verify(Command& cmd, Extra& extra);

  protected:
    // 帧头缓冲区
    Sync<Request>             _buf;
    // 附加参数缓冲区
    Extra                     _extra;
    // 属性值容器
    const PropertyHolderBase& _holder;
};
