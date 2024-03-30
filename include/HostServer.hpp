#include <HostBase.hpp>

struct HostServer : public HostBase
{
    // 是否在特权模式
    bool           privileged = false;
    // 帧头缓冲区
    Request        _req;
    // 附加参数缓冲区
    uint8_t        _extra[UINT8_MAX];
    // 属性值
    PropertyHolder _props;

    /* 轮询请求 */
    virtual bool   poll() override;
    /* 添加属性值 */
    bool           insert(uint16_t id, PropertyBase* prop);
    /* 接收请求帧 */
    bool           recv_request(Command& cmd, uint8_t** extra, uint8_t& size);
    /* 发送响应帧 */
    void           send_response(const Command cmd, const ErrorCode err, const uint8_t* extra, const uint8_t size);
    /* 帧解码 */
    bool           _decode_req(Command& cmd, uint8_t* extra, uint8_t& sizex);
    /* 帧参数解析 */
    bool           _get_property(const Command cmd, uint8_t** extra, uint8_t& size, PropertyBase*& prop);
    bool           _get_memory_param(const Command       cmd,
                                     const PropertyBase* prop,
                                     uint8_t**           extra,
                                     uint8_t&            size,
                                     uint16_t&           offset,
                                     uint8_t&            datlen);
    /* 权限检查 */
    bool           _check_access(const Command cmd, const PropertyBase* prop);
    ErrorCode      _check_read(const PropertyBase* prop) const;
    ErrorCode      _check_write(const PropertyBase* prop) const;
};
