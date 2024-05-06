#pragma once
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <HostBase.hpp>

struct HostClient : public HostBase
{
    // 帧头缓冲区
    Sync<Response> _buf;
    // 帧头
    Response       _rep;
    // 附加参数缓冲区
    Extra          _extra;

    /* 轮询响应 */
    virtual bool   poll() override;
    /* 发送请求帧 */
    void           send_request(const Command cmd, Extra& extra);
    /* 接收响应帧 */
    bool           recv_response(Command& cmd, ErrorCode& err, Extra& extra);
};
