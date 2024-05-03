#include "HostServer.hpp"

#include <array>
#include <checksum.h>

/**
 * @brief 从机轮询主机请求
 *
 * @return true 成功处理一帧
 * @return false 没有接收一帧/帧无效
 */
bool HostServer::poll()
{
    Command cmd;
    Extra&  extra = _extra;
    if (!recv_request(cmd, extra)) return false;

    bool privileged = false;
    // 检查加密标记
    if (IS_ENCRYPTED(cmd))
    {
        extra.encrypted() = true;
        privileged        = extra.decrypt(PropertyBase::Key);
    }

    switch (REMOVE_ENCRYPT_MARK(cmd))
    {
    case Command::ECHO:
    {
        // 读取数据, 防止截断
        extra.seek(extra.remain());
        // 将收到的数据再发回去
        send_response(cmd, ErrorCode::S_OK, extra);
        return true;
    }
    case Command::GET_PROPERTY:
    {
        // 解析Id
        PropertyId id;
        extra.get(id);
        // 查找属性值
        PropertyBase* prop;
        if (!(prop = get(id)))
        {
            send_response(cmd, ErrorCode::E_ID_NOT_EXIST, extra);
            return false;
        }
        // 检查权限
        ErrorCode err;
        err = prop->check_read(privileged);
        if (err != ErrorCode::S_OK)
        {
            send_response(cmd, err, extra);
            return false;
        }
        // 读取属性值
        err = prop->get(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    }
    return false;
}

/**
 * @brief 添加属性值
 *
 * @param id 属性值Id
 * @param prop 属性值
 * @return true 成功添加
 * @return false id重复
 */
bool HostServer::put(PropertyId id, PropertyBase& prop)
{
    bool ok;
    std::tie(std::ignore, ok) = _props.emplace(id, &prop);
    return ok;
}

/**
 * @brief 获取属性值
 *
 * @param id 属性值Id
 * @return PropertyBase* 属性值
 */
PropertyBase* HostServer::get(PropertyId id)
{
    auto it = _props.find(id);
    if (it == _props.end()) return nullptr;
    return (*it).second;
}

/**
 * @brief 接收请求
 *
 * @param cmd [out]接收到的命令
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostServer::recv_request(Command& cmd, Extra& extra)
{
    extra.reset();
    return _decode_req(cmd, extra);
}

/**
 * @brief 发送响应帧
 *
 * @param cmd 响应的指令
 * @param err 错误码
 * @param extra 附加参数
 */
void HostServer::send_response(const Command cmd, const ErrorCode err, Extra& extra)
{
    // 截断多余的数据
    extra.truncate();
    // 若有加密标记, 则对参数进行加密
    if (IS_ENCRYPTED(cmd)) extra.encrypt(PropertyBase::Key);
    Response rep;
    rep.address = address;
    rep.cmd     = cmd;
    rep.error   = err;
    rep.size    = extra.size();
    _encode((uint8_t*)&rep, sizeof(rep), &extra, rep.size);
}

/**
 * @brief 解码请求帧
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostServer::_decode_req(Command& cmd, Extra& extra)
{
    // 解码帧头
    if (!_decode_head((uint8_t*)&_req, sizeof(_req))) return false;

    uint8_t& size = extra.size();
    cmd           = _req.cmd;
    size          = _req.size;

    // 数据为空, 跳过接收
    if (size == 0) return true;

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        extra[i] = rx();
    }
    // 验证数据
    if (crc_ccitt_ffff(&extra, size + sizeof(Chksum)) != 0) return false;

    return true;
}
