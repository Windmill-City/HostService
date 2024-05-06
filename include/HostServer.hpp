#pragma once
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <HostBase.hpp>
#include <map>
#include <Struct.hpp>

using PropertyHolder = std::map<PropertyId, PropertyBase*>;

struct HostServer;

struct PropertyIds : public PropertyAccess<Access::READ>
{
    const HostServer* server;

    explicit PropertyIds(const HostServer* server)
        : server(server)
    {
        this->name = "prop.ids";
    }

    virtual ErrorCode get_mem(Extra& extra) override;
    virtual ErrorCode get_size(Extra& extra) override;
};

struct PropertyNonce : public Struct<NonceType, Access::READ>
{
    PropertyNonce()
    {
        this->name = "prop.nonce";
    }
};

struct PropertyKey : public Struct<KeyType, Access::READ_WRITE_PROTECT>
{
    PropertyKey()
    {
        this->name = "prop.key";
    }

    virtual ErrorCode get(Extra& extra) override
    {
        // 不允许回读密钥
        return ErrorCode::E_NO_IMPLEMENT;
    }
};

struct HostServer : public HostBase
{
    // 帧头缓冲区
    Sync<Request>  _buf;
    // 附加参数缓冲区
    Extra          _extra;
    // 属性值容器
    PropertyHolder _props;

    // Id信息
    PropertyIds    Ids{this};
    // 加密通信随机数, 用来防止重放攻击
    // 每次成功接收一个加密数据包, 就更新此随机数
    // 如果MCU没有随机数外设, 可以使用加密后的唯一ID作为静态随机数
    // 目的是防止不同设备间的重放攻击
    PropertyNonce  Nonce;
    // 加密通信密钥
    PropertyKey    Key;

    HostServer();
    /* 轮询请求 */
    virtual bool  poll() override;
    /* 添加属性值 */
    bool          put(PropertyId id, PropertyBase& prop);
    /* 获取属性值 */
    PropertyBase* get(PropertyId id);
    /* 接收请求帧 */
    bool          recv_request(Command& cmd, Extra& extra);
    /* 发送响应帧 */
    void          send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 属性值获取与鉴权 */
    PropertyBase* _acquire_and_verify(Command& cmd, Extra& extra);
};
