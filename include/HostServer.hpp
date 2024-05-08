#pragma once
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <frozen/map.h>
#include <HostBase.hpp>
#include <Struct.hpp>

using PropertyNonce = Struct<NonceType, Access::READ>;

struct PropertyKey : public Struct<KeyType, Access::READ_WRITE_PROTECT>
{
    virtual ErrorCode get(Extra& extra) override
    {
        // 不允许回读密钥
        return ErrorCode::E_NO_IMPLEMENT;
    }
};

struct HostServerBase : public HostBase
{
    // 帧头缓冲区
    Sync<Request>         _buf;
    // 附加参数缓冲区
    Extra                 _extra;

    // 加密通信随机数, 用来防止重放攻击
    // 每次成功接收一个加密数据包, 就更新此随机数
    // 如果MCU没有随机数外设, 可以使用加密后的唯一ID作为静态随机数
    // 可以防止不同设备间的重放攻击
    PropertyNonce         Nonce;
    // 加密通信密钥
    PropertyKey           Key;

    virtual bool          poll() override;
    bool                  recv_request(Command& cmd, Extra& extra);
    void                  send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 属性值获取与鉴权 */
    PropertyBase*         _acquire_and_verify(Command& cmd, Extra& extra);
    /* 获取属性值 */
    virtual PropertyBase* get(PropertyId id) = 0;
    /* 更新随机数 */
    virtual void          update_nonce()     = 0;
};

template <size_t _size>
struct HostServer : public HostServerBase
{
    using PropertyHolder     = frozen::map<PropertyId, PropertyBase*, _size>;
    using PropertyHolderInit = frozen::map<PropertyId, PropertyBase*, _size>::container_type;

    const PropertyHolder _props;
};