#include "HostClient.hpp"

#include <checksum.h>

/**
 * @brief 主机轮询从机应答
 *
 * @return true 成功接收一帧
 * @return false 没有接收一帧/帧无效
 */
bool HostClient::poll()
{
    Command   cmd;
    Extra&    extra = _extra;
    ErrorCode err;
    uint8_t   size;
    if (!recv_response(cmd, err, extra)) return false;

    switch (cmd)
    {
    case Command::ECHO:
        break;
    case Command::GET_SIZE:
    case Command::GET_PROPERTY:
    case Command::SET_PROPERTY:
        break;
    case Command::GET_MEMORY:
    case Command::SET_MEMORY:
        break;
    }
    return true;
}

/**
 * @brief 发送请求
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 */
void HostClient::send_request(const Command cmd, Extra& extra)
{
    Request req;
    req.address = address;
    req.cmd     = extra.encrypted() ? ADD_ENCRYPT_MARK(cmd) : cmd;
    req.size    = extra.size();
    _encode((uint8_t*)&req, sizeof(req), &extra, req.size);
}

/**
 * @brief 接收响应
 *
 * @param cmd [out]响应的命令
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostClient::recv_response(Command& cmd, ErrorCode& err, Extra& extra)
{
    extra.reset();
    return _decode_rep(cmd, err, extra);
}

/**
 * @brief 解码响应帧
 *
 * @param cmd 响应的指令
 * @param err 错误码
 * @param extra 附加参数
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostClient::_decode_rep(Command& cmd, ErrorCode& err, Extra& extra)
{
    _buf.push(rx());
    if (!_buf.verify<Response>()) return false;

    _rep          = _buf.as<Response>();

    uint8_t& size = extra.size();
    cmd           = _rep.cmd;
    size          = _rep.size;
    err           = _rep.error;

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