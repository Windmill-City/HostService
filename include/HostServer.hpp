#pragma once
#include <Extra.hpp>
#include <HostBase.hpp>
#include <map>
#include <Struct.hpp>

using PropertyHolder = std::map<PropertyId, PropertyBase*>;

struct HostServer;

struct PropertyIds : public PropertyAccess<Access::READ>
{
    const HostServer* server;

    PropertyIds(const HostServer* server);

    virtual ErrorCode get_mem(Extra& extra) override;
    virtual ErrorCode get_size(Extra& extra) override;
};

struct PropertyNonce : public Struct<NonceType, Access::READ>
{
    using parent = Struct<NonceType, Access::READ>;
    PropertyNonce();
};

struct PropertyKey : public Struct<std::array<uint8_t, 256 / 8>, Access::READ_WRITE_PROTECT>
{
    using parent = Struct<std::array<uint8_t, 256 / 8>, Access::READ_WRITE_PROTECT>;
    PropertyKey();
    virtual ErrorCode get(Extra& extra) override;
};

struct HostServer : public HostBase
{
    // 帧头缓冲区
    Request        _req;
    // 附加参数缓冲区
    Extra          _extra;
    // 属性值容器
    PropertyHolder _props;

    // Id信息
    PropertyIds    Ids{this};
    // 加密通信随机数
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
    /* 帧解码 */
    bool          _decode_req(Command& cmd, Extra& extra);
    /* 属性值获取与校验 */
    PropertyBase* _acquire_and_verify(Command& cmd, Extra& extra);
};
