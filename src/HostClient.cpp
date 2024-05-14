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
    Extra&    extra = this->extra;
    ErrorCode err;
    if (!recv_response(cmd, err, extra)) return false;

    switch (cmd)
    {
    case Command::ECHO:
        break;
    case Command::GET_SIZE:
    case Command::GET_PROPERTY:
    case Command::SET_PROPERTY:
        break;
    case Command::SIGNAL:
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
    while (_buf.size() < sizeof(Response))
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf.push(byte);
    }
    while (!_buf.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf.push(byte);
    }

    extra.reset();
    rep            = _buf.get();

    uint16_t& size = extra.size();
    cmd            = rep.cmd;
    size           = rep.size;
    err            = rep.error;

    // 数据为空, 跳过接收
    if (size == 0) return true;

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        extra[i] = byte;
    }
    // 验证数据
    if (crc_ccitt_ffff(&extra, size + sizeof(Chksum)) != 0) return false;

    return true;
}
