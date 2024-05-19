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
    req.cmd     = extra.encrypted() ? ADD_ENCRYPT_MARK(cmd) : REMOVE_ENCRYPT_MARK(cmd);
    req.size    = extra.size();
    if (extra.encrypted())
        _encode((uint8_t*)&req, sizeof(req), extra.tag(), extra.size() + sizeof(TagType));
    else
        _encode((uint8_t*)&req, sizeof(req), extra.data(), extra.size());
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
    // 帧同步
    while (!_buf.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf.push(byte);
    }

    Response rep = _buf.get();
    cmd          = REMOVE_ENCRYPT_MARK(rep.cmd);
    err          = rep.error;
    extra.reset();
    extra.size()      = rep.size;
    extra.encrypted() = IS_ENCRYPTED(rep.cmd) && extra.size() > 0;

    // 数据长度为 0 则跳过读取
    if (extra.size() == 0)
        goto End;
    else
    {
        uint16_t chksum = CRC_START_CCITT_FFFF;
        // 读取tag
        if (extra.encrypted())
        {
            for (size_t i = 0; i < sizeof(TagType); i++)
            {
                uint8_t byte;
                if (!rx(byte)) return false; // 接收超时
                chksum         = update_crc_ccitt(chksum, byte);
                extra.tag()[i] = byte;
            }
        }

        // 读取数据
        for (size_t i = 0; i < extra.size(); i++)
        {
            uint8_t byte;
            if (!rx(byte)) return false; // 接收超时
            chksum          = update_crc_ccitt(chksum, byte);
            extra.data()[i] = byte;
        }

        // 读取校验和
        for (size_t i = 0; i < sizeof(chksum); i++)
        {
            uint8_t byte;
            if (!rx(byte)) return false; // 接收超时
            chksum = update_crc_ccitt(chksum, byte);
        }

        // 验证数据
        if (chksum != 0) goto Start;
    }
End:
    // 验证地址
    if (rep.address != address) goto Start;

    return true;
}
