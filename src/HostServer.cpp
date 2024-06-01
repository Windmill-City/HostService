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
    bool      encrypted;
    Command   cmd;
    ErrorCode err;
    Extra&    extra = _extra;
    if (!recv(cmd, err, extra)) return false;

    // 检查加密标记
    if (encrypted = extra.encrypted())
    {
        if (!extra.decrypt(secret.nonce, secret.key))
        {
            send(cmd, extra, false, ErrorCode::E_INVALID_ARG);
            return false;
        }
    }

    switch (cmd)
    {
    case Command::ECHO:
    {
        // 读取全部数据, 防止截断
        extra.readall();
        // 将收到的数据再发回去
        err = ErrorCode::S_OK;
        send(cmd, extra, encrypted, err);
        break;
    }
    case Command::GET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra, encrypted))) return false;
        // 读取属性值
        err = prop->get(extra);
        send(cmd, extra, encrypted, err);
        break;
    }
    case Command::SET_PROPERTY:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra, encrypted))) return false;
        // 写入属性值
        err = prop->set(extra);
        send(cmd, extra, encrypted, err);
        break;
    }
    case Command::GET_SIZE:
    {
        PropertyBase* prop;
        if (!(prop = _acquire_and_verify(cmd, extra, encrypted))) return false;
        // 读取Size
        err = prop->get_size(extra);
        send(cmd, extra, encrypted, err);
        break;
    }
    default:
        err = ErrorCode::E_NO_IMPLEMENT;
        send(cmd, extra, encrypted, err);
        break;
    }

    // 发送响应后再更新随机数
    if (encrypted) secret.update_nonce();
    return err == ErrorCode::S_OK;
}

/**
 * @brief 发送 Server log
 *
 * @param log log信息
 * @param size log大小
 */
void HostServer::send_log(const void* log, const size_t size)
{
    Header head;
    head.address = address;
    head.cmd     = Command::LOG;
    head.error   = ErrorCode::S_OK;
    head.size    = size;
    send(head, log, size);
}

PropertyBase* HostServer::_acquire_and_verify(Command& cmd, Extra& extra, const bool encrypted)
{
    // 解析Id
    PropertyId id;
    if (!extra.get(id))
    {
        send(cmd, extra, encrypted, ErrorCode::E_INVALID_ARG);
        return nullptr;
    }
    // 查找属性值
    PropertyBase* prop;
    if (!(prop = _holder.get(id)))
    {
        send(cmd, extra, encrypted, ErrorCode::E_ID_NOT_EXIST);
        return nullptr;
    }
    // 检查权限
    ErrorCode err;
    switch (cmd)
    {
    case Command::GET_SIZE:
    case Command::GET_PROPERTY:
        err = prop->check_read(encrypted);
        break;
    case Command::SET_PROPERTY:
        err = prop->check_write(encrypted);
        break;
    default:
        err = ErrorCode::S_OK;
        break;
    }
    if (err != ErrorCode::S_OK)
    {
        send(cmd, extra, encrypted, err);
        return nullptr;
    }
    return prop;
}
