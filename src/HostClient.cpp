#include "HostClient.hpp"

#include <checksum.h>

/**
 * @brief 发送请求
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 */
void HostClient::send_request(const Command cmd, Extra& extra, bool encrypt)
{
    if (encrypt) extra.encrypt(nonce, key);
    Request req;
    req.address = address;
    req.cmd     = encrypt || extra.encrypted() ? ADD_ENCRYPT_MARK(cmd) : cmd;
    req.size    = extra.size();
    _encode((uint8_t*)&req, sizeof(req), &extra, req.size);
}

/**
 * @brief 接收响应
 *
 * @param cmd [out]期望响应的命令
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostClient::recv_response(Command cmd, ErrorCode& err, Extra& extra)
{
Start:
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
    Response  rep  = _buf.get();

    uint16_t& size = extra.size();
    size           = rep.size;
    err            = rep.error;

    // 数据为空, 跳过接收
    if (size == 0)
    {
        // 验证地址
        if (rep.address != address) goto Start;
        // 验证指令
        if (REMOVE_ENCRYPT_MARK(rep.cmd) != cmd) goto Start;
        return true;
    }

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        extra[i] = byte;
    }
    // 验证数据
    if (crc_ccitt_ffff(&extra, size + sizeof(Chksum)) != 0) goto Start;

    // 验证地址
    if (rep.address != address) goto Start;
    // 验证指令
    if (REMOVE_ENCRYPT_MARK(rep.cmd) != cmd) goto Start;

    // 检查加密标记
    if (IS_ENCRYPTED(cmd))
    {
        extra.encrypted() = true;
        if (!extra.decrypt(nonce, key)) goto Start;
    }

    // 去除加密标记, 方便后续处理
    rep.cmd = REMOVE_ENCRYPT_MARK(cmd);

    return true;
}
