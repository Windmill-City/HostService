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

    HostClient(const PropertyAddress& addr)
        : HostBase(addr)
    {
    }

    virtual bool poll() override;
    void         send_request(const Command cmd, Extra& extra);
    bool         recv_response(Command& cmd, ErrorCode& err, Extra& extra);
};
