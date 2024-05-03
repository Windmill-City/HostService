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

    // 检查加密标记
    if (IS_ENCRYPTED(cmd))
    {
        extra.encrypted() = true;
        if (!extra.decrypt(PropertyBase::Key))
        {
            // 清空附加参数
            extra.reset();
            send_response(cmd, ErrorCode::E_INVALID_ARG, extra);
            return false;
        }
    }

    switch (REMOVE_ENCRYPT_MARK(cmd))
    {
    case Command::ECHO:
    {
        // 读取全部数据, 防止截断
        extra.readall();
        // 将收到的数据再发回去
        send_response(cmd, ErrorCode::S_OK, extra);
        return true;
    }
    case Command::GET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取属性值
        ErrorCode err = prop->get(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    case Command::SET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 写入属性值
        ErrorCode err = prop->set(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    case Command::GET_MEMORY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取内存
        ErrorCode err = prop->get_mem(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    case Command::SET_MEMORY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 写入内存
        ErrorCode err = prop->set_mem(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    case Command::GET_SIZE:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取Size
        ErrorCode err = prop->get_size(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    case Command::GET_DESC:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取描述
        ErrorCode err = prop->get_desc(extra);
        send_response(cmd, err, extra);
        return err == ErrorCode::S_OK;
    }
    default:
        send_response(cmd, ErrorCode::E_NO_IMPLEMENT, extra);
        return false;
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

PropertyBase* HostServer::_acquire_and_verify(Command& cmd, Extra& extra)
{
    // 解析Id
    PropertyId id;
    extra.get(id);
    // 查找属性值
    PropertyBase* prop;
    if (!(prop = get(id)))
    {
        send_response(cmd, ErrorCode::E_ID_NOT_EXIST, extra);
        return nullptr;
    }
    // 检查权限
    ErrorCode err;
    switch (REMOVE_ENCRYPT_MARK(cmd))
    {
    case Command::GET_DESC:
    case Command::GET_SIZE:
    case Command::GET_MEMORY:
    case Command::GET_PROPERTY:
        err = prop->check_read(IS_ENCRYPTED(cmd));
        break;
    case Command::SET_MEMORY:
    case Command::SET_PROPERTY:
        err = prop->check_write(IS_ENCRYPTED(cmd));
        break;
    default:
        err = ErrorCode::S_OK;
        break;
    }
    if (err != ErrorCode::S_OK)
    {
        send_response(cmd, err, extra);
        return nullptr;
    }
    return prop;
}
