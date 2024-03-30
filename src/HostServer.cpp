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
    Command  cmd;
    uint8_t* extra;
    uint8_t  size;
    if (!recv_request(cmd, &extra, size)) return false;

    switch (cmd)
    {
    case Command::ECHO:
    {
        // 将收到的数据再发回去
        send_response(cmd, ErrorCode::S_OK, extra, size);
        break;
    }
    case Command::GET_PROPERTY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, &extra, size, prop)) return false;

        uint8_t*  p;
        uint8_t   size;
        ErrorCode code = prop->get(&p, size);
        send_response(cmd, code, p, size);
        break;
    }
    case Command::SET_PROPERTY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, &extra, size, prop)) return false;

        ErrorCode code = prop->set(extra, size);
        send_response(cmd, code, NULL, 0);
        return code == ErrorCode::S_OK;
    }
    case Command::GET_MEMORY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, &extra, size, prop)) return false;

        uint16_t offset;
        uint8_t  datlen;
        if (!_get_memory_param(cmd, prop, &extra, size, offset, datlen)) return false;

        uint8_t*  p;
        uint8_t   size;
        ErrorCode code = prop->get_mem(offset, &p, size);
        send_response(cmd, code, p, size);
        break;
    }
    case Command::SET_MEMORY:
    {
        PropertyBase* prop;
        if (!_get_property(cmd, &extra, size, prop)) return false;

        // 获取内存访问参数
        uint16_t offset;
        uint8_t  datlen;
        if (!_get_memory_param(cmd, prop, &extra, size, offset, datlen)) return false;

        // 设置属性值, 并发送返回码
        ErrorCode code = prop->set_mem(offset, extra, datlen);
        send_response(cmd, code, NULL, 0);
        return code == ErrorCode::S_OK;
    }
    }
    return true;
}

/**
 * @brief 添加受管理的属性值
 *
 * @param id 属性Id
 * @param prop 属性值
 * @return true 添加成功
 * @return false Id重复
 */
bool HostServer::insert(uint16_t id, PropertyBase* prop)
{
    bool ok;
    std::tie(std::ignore, ok) = _props.emplace(id, prop);
    return ok;
}

/**
 * @brief 接收请求
 *
 * @param cmd [out]接收到的命令
 * @param extra [out]附加参数数组的指针, 调用者无需释放此指针
 * @param size [out]参数长度
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 *
 * 注意: 接收响应和接收请求共用一个缓冲区
 */
bool HostServer::recv_request(Command& cmd, uint8_t** extra, uint8_t& size)
{
    if (extra) *extra = _extra;
    size = 0;
    return _decode_req(cmd, *extra, size);
}

/**
 * @brief 发送响应帧
 *
 * @param cmd 响应的指令
 * @param err 错误码
 * @param extra 附加参数
 * @param size 参数长度
 */
void HostServer::send_response(const Command cmd, const ErrorCode err, const uint8_t* extra, const uint8_t size)
{
    Response rep;
    rep.address = address;
    rep.cmd     = cmd;
    rep.error   = err;
    rep.size    = size;
    _encode((uint8_t*)&rep, sizeof(rep), extra, size);
}

/**
 * @brief 解码请求帧
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @param size 附加参数长度
 * @param rx 数据接收方法
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostServer::_decode_req(Command& cmd, uint8_t* extra, uint8_t& size)
{
    // 解码帧头
    if (!_decode_head((uint8_t*)&_req, sizeof(_req))) return false;

    cmd  = _req.cmd;
    size = _req.size;

    // 数据为空, 跳过接收
    if (size == 0) return true;

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        extra[i] = rx();
    }
    // 验证数据
    if (crc_ccitt_ffff(extra, size + sizeof(Chksum)) != 0) return false;

    return true;
}

/**
 * @brief 根据附加参数获取属性值
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @param size 参数长度
 * @param prop 接收prop属性指针的变量
 * @return true 成功获取变量
 * @return false 获取变量失败
 */
bool HostServer::_get_property(Command cmd, uint8_t** extra, uint8_t& size, PropertyBase*& prop)
{
    uint16_t id;
    // 检查长度是否过短
    if (size < sizeof(id))
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, NULL, 0);
        return false;
    }
    // 读取id
    id = *(uint16_t*)*extra;
    size -= sizeof(id);
    *extra += sizeof(id);

    // 通过id从map中取出属性值
    auto it = _props.find(id);
    if (it == _props.end())
    {
        send_response(cmd, ErrorCode::E_ID_NOT_EXIST, NULL, 0);
        return false;
    }
    prop = (*it).second;
    // 检查访问权限
    return _check_access(cmd, prop);
}

/**
 * @brief 根据附加参数获取内存访问的参数
 *
 * @param cmd 请求的指令
 * @param prop 内存属性值
 * @param extra 附加参数
 * @param size 参数长度
 * @param offset 地址偏移
 * @param datlen 数据长度
 * @return true 成功获取变量
 * @return false 获取变量失败
 */
bool HostServer::_get_memory_param(const Command       cmd,
                                   const PropertyBase* prop,
                                   uint8_t**           extra,
                                   uint8_t&            size,
                                   uint16_t&           offset,
                                   uint8_t&            datlen)
{
    // 检查数据长度是否过短
    if (size < sizeof(offset) + sizeof(datlen))
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, NULL, 0);
        return false;
    }

    // 写入偏移
    offset = *(uint16_t*)(*extra);
    *extra += sizeof(offset);
    size -= sizeof(offset);

    // 写入长度
    datlen = **extra;
    *extra += sizeof(datlen);
    size -= sizeof(datlen);

    // datlen用于防止漏传offset造成的内存意外写入
    if (cmd == Command::SET_MEMORY && size != datlen)
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, NULL, 0);
        return false;
    }

    return true;
}

/**
 * @brief 检查指令是否有权限读取/写入变量
 *
 * @param cmd 需要检查的指令
 * @param prop 访问的变量
 * @return true 有权限
 * @return false 无权限
 */
bool HostServer::_check_access(const Command cmd, const PropertyBase* prop)
{
    ErrorCode code;
    switch (cmd)
    {
    case Command::GET_SIZE:
    case Command::GET_MEMORY:
    case Command::GET_PROPERTY:
    {
        // 检查读取权限
        if ((code = _check_read(prop)) != ErrorCode::S_OK)
        {
            send_response(cmd, code, NULL, 0);
            return false;
        }
        break;
    }
    case Command::SET_MEMORY:
    case Command::SET_PROPERTY:
    {
        // 检查写入权限
        if ((code = _check_write(prop)) != ErrorCode::S_OK)
        {
            send_response(cmd, code, NULL, 0);
            return false;
        }
        break;
    }
    default:
        break;
    }
    return true;
}

/**
 * @brief 检查读权限
 *
 * @return ErrorCode 错误码
 */
ErrorCode HostServer::_check_read(const PropertyBase* prop) const
{
    if (prop->access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
    if (prop->access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
    return ErrorCode::S_OK;
}

/**
 * @brief 检查写权限
 *
 * @return ErrorCode 错误码
 */
ErrorCode HostServer::_check_write(const PropertyBase* prop) const
{
    if (prop->access == Access::READ) return ErrorCode::E_READ_ONLY;
    if (prop->access == Access::READ_PROTECT && privileged) return ErrorCode::E_READ_ONLY;
    if (prop->access == Access::READ_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
    if (prop->access == Access::WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
    if (prop->access == Access::READ_WRITE_PROTECT && !privileged) return ErrorCode::E_NO_PERMISSION;
    return ErrorCode::S_OK;
}