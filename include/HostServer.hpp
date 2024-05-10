#pragma once
#include <algorithm>
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <frozen/string.h>
#include <HostBase.hpp>
#include <Struct.hpp>

template <size_t _size>
using PropertyMap   = std::array<std::pair<frozen::string, PropertyBase*>, _size>;
using PropertyNonce = Struct<NonceType, Access::READ>;

struct PropertyKey : public Struct<KeyType, Access::READ_WRITE_PROTECT>
{
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
    PropertyHolderBase* holder = nullptr;

    virtual ErrorCode   get(Extra& extra) override
    {
        PropertyId id;
        if (!extra.get(id)) return ErrorCode::E_INVALID_ARG;
        if (id >= holder->size()) return ErrorCode::E_OUT_OF_INDEX;

        frozen::string desc = holder->get_desc(id);
        extra.add(desc.data(), desc.size());
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        if (holder->size() > UINT16_MAX) return ErrorCode::E_OUT_OF_INDEX;
        extra.add<uint16_t>(holder->size());
        return ErrorCode::S_OK;
    }
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
        ids.holder = this;
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
    HostServer(const PropertyHolderBase& holder, SecretHolder& secret)
        : _secret(secret)
        , _holder(holder)
    {
    }

    virtual bool  poll() override;
    bool          recv_request(Command& cmd, Extra& extra);
    void          send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 属性值获取与鉴权 */
    PropertyBase* _acquire_and_verify(Command& cmd, Extra& extra);

  public:
    // 帧头缓冲区
    Sync<Request>             _buf;
    // 附加参数缓冲区
    Extra                     _extra;
    // 密钥容器
    SecretHolder&             _secret;
    // 属性值容器
    const PropertyHolderBase& _holder;
};
