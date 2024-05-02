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
    }
    return false;
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
