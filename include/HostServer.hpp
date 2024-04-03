#include <Extra.hpp>
#include <HostBase.hpp>

struct HostServer : public HostBase
{
    // 是否在特权模式
    bool           privileged = false;
    // 帧头缓冲区
    Request        _req;
    // 附加参数缓冲区
    Extra          _extra;
    // 属性值
    PropertyHolder _props;

    /* 轮询请求 */
    virtual bool   poll() override;
    /* 添加属性值 */
    bool           insert(uint16_t id, PropertyBase& prop);
    /* 接收请求帧 */
    bool           recv_request(Command& cmd, Extra& extra);
    /* 发送响应帧 */
    void           send_response(const Command cmd, const ErrorCode err, Extra& extra);
    /* 帧解码 */
    bool           _decode_req(Command& cmd, Extra& extra);
    /* 帧参数解析 */
    bool           _get_property(const Command cmd, Extra& extra, PropertyBase** prop);
    bool           _get_memory(const Command cmd, const PropertyBase* prop, Extra& extra);
    /* 权限检查 */
    ErrorCode      _check_access(const Command cmd, const PropertyBase* prop);
};
