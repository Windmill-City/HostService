#include "HostServer.hpp"

#include <array>
#include <checksum.h>

HostServer::HostServer()
{
    put(0, this->Ids);
    put(1, this->Nonce);
    put(2, this->Key);
}

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
        if (!extra.decrypt(Nonce, Key))
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
    _buf.push(rx());
    if (!_buf.verify()) return false;

    extra.reset();
    Request  _req = _buf.get();

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
    if (IS_ENCRYPTED(cmd)) extra.encrypt(Nonce, Key);
    Response rep;
    rep.address = address;
    rep.cmd     = cmd;
    rep.error   = err;
    rep.size    = extra.size();
    _encode((uint8_t*)&rep, sizeof(rep), &extra, rep.size);
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

PropertyIds::PropertyIds(const HostServer* server)
    : server(server)
{
    this->name = "prop.ids";
}

ErrorCode PropertyIds::get_mem(Extra& extra)
{
    MemoryAccess access;
    // 检查访问参数是否正确
    if (!extra.get(access) && access.offset % sizeof(PropertyId) == 0 && access.size % sizeof(PropertyId) == 0)
        return ErrorCode::E_INVALID_ARG;
    // 检查是否超出内存区范围
    size_t size = server->_props.size() * sizeof(PropertyId);
    if (size < access.offset + access.size)
    {
        return ErrorCode::E_OUT_OF_INDEX;
    }

    // 需要获取的元素的数量
    size_t count = access.size / sizeof(PropertyId);
    // 从第几个元素开始读取
    size_t i_beg = access.offset / sizeof(PropertyId);
    size_t i     = 0;
    for (auto it : server->_props)
    {
        if (i++ >= i_beg)
        {
            if (!extra.add(it.first)) return ErrorCode::E_OUT_OF_BUFFER;
            if (--count == 0) break;
        }
    }
    return ErrorCode::S_OK;
}

ErrorCode PropertyIds::get_size(Extra& extra)
{
    size_t size = server->_props.size() * sizeof(PropertyId);
    extra.add<uint16_t>(size);
    return ErrorCode::S_OK;
}
