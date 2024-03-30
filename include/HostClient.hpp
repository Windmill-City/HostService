#pragma once
#include <HostBase.hpp>

struct HostClient : public HostBase
{
    // 帧头缓冲区
    Response     _rep;
    // 附加参数缓冲区
    uint8_t      _extra[UINT8_MAX];

    /* 轮询响应 */
    virtual bool poll() override;
    /* 发送请求帧 */
    void         send_request(const Command cmd, const uint8_t* extra, const uint8_t size);
    /* 接收响应帧 */
    bool         recv_response(Command& cmd, ErrorCode& err, uint8_t** extra, uint8_t& size);
    /* 帧解码 */
    bool         _decode_rep(Command& cmd, ErrorCode& err, uint8_t* extra, uint8_t& size);
};
