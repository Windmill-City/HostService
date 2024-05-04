#pragma once
#include <Extra.hpp>
#include <HostBase.hpp>
#include <map>
#include <PropertyBase.hpp>

using PropertyHolder = std::map<PropertyId, PropertyBase*>;

struct HostServer;

struct PropertyIds : public PropertyAccess<Access::READ>
{
    const HostServer* server;

    PropertyIds(const HostServer* server);

    virtual ErrorCode get_mem(Extra& extra) override;
    virtual ErrorCode get_size(Extra& extra) override;
};

struct PropertyNonce : public PropertyAccess<Access::READ>
{
    PropertyNonce();
    virtual ErrorCode get(Extra& extra) override;
    virtual ErrorCode get_size(Extra& extra) override;
};

struct HostServer : public HostBase
{
    // 帧头缓冲区
    Request        _req;
    // 附加参数缓冲区
    Extra          _extra;
    // 属性值容器
    PropertyHolder _props;

    // 属性值Id信息
    PropertyIds    Ids{this};
    // 属性值加密通信随机数
    PropertyNonce  Nonce;

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
