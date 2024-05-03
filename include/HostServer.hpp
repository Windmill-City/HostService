#pragma once
#include <Extra.hpp>
#include <HostBase.hpp>
#include <map>
#include <PropertyBase.hpp>

using PropertyHolder = std::map<PropertyId, PropertyBase*>;

struct HostServer : public HostBase
{
    // 帧头缓冲区
    Request        _req;
    // 附加参数缓冲区
    Extra          _extra;
    // 属性值容器
    PropertyHolder _props;

    /* 轮询请求 */
    virtual bool   poll() override;
    /* 添加/获取属性值 */
    bool           put(PropertyId id, PropertyBase& prop);
    PropertyBase*  get(PropertyId id);
    /* 接收请求帧 */
    bool           recv_request(Command& cmd, Extra& extra);
    /* 发送响应帧 */
    void           send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 帧解码 */
    bool           _decode_req(Command& cmd, Extra& extra);
};
