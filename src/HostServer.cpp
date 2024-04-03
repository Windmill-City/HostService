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

    switch (cmd)
    {
    case Command::ECHO:
    {
        // 将收到的数据再发回去
        send_response(cmd, ErrorCode::S_OK, extra);
        return true;
    }
    case Command::GET_PROPERTY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, extra, &prop)) return false;
        extra.truncate(); // 截断多余数据

        ErrorCode code = prop->get(extra);
        send_response(cmd, code, extra);
        return code == ErrorCode::S_OK;
    }
    case Command::SET_PROPERTY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, extra, &prop)) return false;

        ErrorCode code = prop->set(extra);
        send_response(cmd, code, extra);
        return code == ErrorCode::S_OK;
    }
    case Command::GET_MEMORY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, extra, &prop)) return false;
        if (!_get_memory(cmd, prop, extra)) return false;
        extra.truncate(); // 截断多余数据

        ErrorCode code = prop->get_mem(extra);
        send_response(cmd, code, extra);
        return code == ErrorCode::S_OK;
    }
    case Command::SET_MEMORY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, extra, &prop)) return false;
        if (!_get_memory(cmd, prop, extra)) return false;

        ErrorCode code = prop->set_mem(extra);
        send_response(cmd, code, extra);
        return code == ErrorCode::S_OK;
    }
    case Command::GET_SIZE:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, extra, &prop)) return false;
        extra.truncate(); // 截断多余的数据

        ErrorCode code = prop->get_size(extra);
        send_response(cmd, code, extra);
        return code == ErrorCode::S_OK;
    }
    }
    return false;
}

/**
 * @brief 添加受管理的属性值
 *
 * @param id 属性Id
 * @param prop 属性值
 * @return true 添加成功
 * @return false Id重复
 */
bool HostServer::insert(uint16_t id, PropertyBase& prop)
{
    bool ok;
    std::tie(std::ignore, ok) = _props.emplace(id, &prop);
    return ok;
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
    // 如果指令执行过程中有错误, 则截断数据区, 减少不必要的数据传输
    if (err != ErrorCode::S_OK) extra.truncate();
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

/**
 * @brief 根据附加参数获取属性值
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @param prop 接收prop属性指针的变量
 * @return true 成功获取变量
 * @return false 获取变量失败
 */
bool HostServer::_get_property(Command cmd, Extra& extra, PropertyBase** prop)
{
    // 解码Id
    if (!extra.decode_id())
    {
        send_response(cmd, ErrorCode::E_ID_NOT_EXIST, extra);
        return false;
    }
    // 通过id从map中取出属性值
    auto it = _props.find(extra.id());
    if (it == _props.end())
    {
        send_response(cmd, ErrorCode::E_ID_NOT_EXIST, extra);
        return false;
    }
    *prop = (*it).second;
    // 检查访问权限
    ErrorCode code;
    if ((code = _check_access(cmd, *prop)) != ErrorCode::S_OK)
    {
        send_response(cmd, code, extra);
        return false;
    }
    return true;
}

/**
 * @brief 根据附加参数获取内存访问的参数
 *
 * @param cmd 请求的指令
 * @param prop 内存属性值
 * @param extra 附加参数
 * @return true 成功获取变量
 * @return false 获取变量失败
 */
bool HostServer::_get_memory(const Command cmd, const PropertyBase* prop, Extra& extra)
{
    // 检查数据长度是否过短
    if (!extra.decode_mem())
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, extra);
        return false;
    }

    // datlen用于防止因漏传offset, 而造成的内存意外写入
    if (cmd == Command::SET_MEMORY && extra.data_size() != extra.datlen())
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, extra);
        return false;
    }
    return true;
}

/**
 * @brief 检查指令是否有权限读取/写入变量
 *
 * @param cmd 需要检查的指令
 * @param prop 访问的变量
 * @return ErrorCode 错误码
 */
ErrorCode HostServer::_check_access(const Command cmd, const PropertyBase* prop)
{
    ErrorCode code;
    switch (cmd)
    {
    case Command::GET_SIZE:
    case Command::GET_MEMORY:
    case Command::GET_PROPERTY:
        return prop->check_read(privileged);
    case Command::SET_MEMORY:
    case Command::SET_PROPERTY:
        return prop->check_write(privileged);
    default:
        break;
    }
    return ErrorCode::S_OK;
}