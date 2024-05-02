#pragma once
#include <Extra.hpp>
#include <HostBase.hpp>

struct HostServer : public HostBase
{
    // 帧头缓冲区
    Request      _req;
    // 附加参数缓冲区
    Extra        _extra;

    /* 轮询请求 */
    virtual bool poll() override;
    /* 接收请求帧 */
    bool         recv_request(Command& cmd, Extra& extra);
    /* 发送响应帧 */
    void         send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 帧解码 */
    bool         _decode_req(Command& cmd, Extra& extra);
};
