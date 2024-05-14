#pragma once
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <HostBase.hpp>

struct HostClient : public HostBase
{
    // 帧头缓冲区
    Sync<Response> _buf;
    // 附加参数缓冲区
    Extra          extra;
    // 随机数
    NonceType      nonce;
    // 通信密钥
    KeyType        key;

    HostClient(const PropertyAddress& addr)
        : HostBase(addr)
    {
    }

    void send_request(const Command cmd, Extra& extra, bool encrypt = false);
    bool recv_response(Command cmd, ErrorCode& err, Extra& extra);
};
