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
    Command   cmd;
    ErrorCode err;
    Extra&    extra = _extra;
    if (!recv_request(cmd, extra)) return false;

    // 检查加密标记
    if (IS_ENCRYPTED(cmd))
    {
        extra.encrypted() = true;
        if (!extra.decrypt(_secret.nonce, _secret.key))
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
        err = ErrorCode::S_OK;
        send_response(cmd, err, extra);
        break;
    }
    case Command::GET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取属性值
        err = prop->get(extra);
        send_response(cmd, err, extra);
        break;
    }
    case Command::SET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 写入属性值
        err = prop->set(extra);
        send_response(cmd, err, extra);
        break;
    }
    case Command::GET_SIZE:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra))) return false;
        // 读取Size
        err = prop->get_size(extra);
        send_response(cmd, err, extra);
        break;
    }
    default:
        err = ErrorCode::E_NO_IMPLEMENT;
        send_response(cmd, err, extra);
        break;
    }

    // 发送响应后再更新随机数
    if (IS_ENCRYPTED(cmd)) _secret.update_nonce();
    return err == ErrorCode::S_OK;
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
    if (size == 0)
    {
        // 验证地址
        if (_req.address != address) return false;
        return true;
    }

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        extra[i] = rx();
    }
    // 验证数据
    if (crc_ccitt_ffff(&extra, size + sizeof(Chksum)) != 0) return false;

    // 验证地址
    if (_req.address != address) return false;

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
    if (IS_ENCRYPTED(cmd)) extra.encrypt(_secret.nonce, _secret.key);
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
    if (!extra.get(id))
    {
        send_response(cmd, ErrorCode::E_INVALID_ARG, extra);
        return nullptr;
    }
    // 查找属性值
    PropertyBase* prop;
    if (!(prop = _holder.get(id)))
    {
        send_response(cmd, ErrorCode::E_ID_NOT_EXIST, extra);
        return nullptr;
    }
    // 检查权限
    ErrorCode err;
    switch (REMOVE_ENCRYPT_MARK(cmd))
    {
    case Command::GET_SIZE:
    case Command::GET_PROPERTY:
        err = prop->check_read(IS_ENCRYPTED(cmd));
        break;
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
