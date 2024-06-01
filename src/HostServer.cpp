#include "HostServer.hpp"

#include <algorithm>
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
    if (extra.encrypted())
    {
        if (!extra.decrypt(secret.nonce, secret.key))
        {
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
    if (IS_ENCRYPTED(cmd)) secret.update_nonce();
    return err == ErrorCode::S_OK;
}

/**
 * @brief 发送 Server log
 *
 * @param log log信息
 * @param size log大小
 */
void HostServer::send_log(const char* log, size_t size)
{
    Response rep;
    rep.address = address;
    rep.cmd     = Command::LOG;
    rep.error   = ErrorCode::S_OK;
    rep.size    = size;
    _encode((uint8_t*)&rep, sizeof(rep), (uint8_t*)log, size);
    recv_ack();
}

void HostServer::log_output(const char* log, const size_t size)
{
    // ignore
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
