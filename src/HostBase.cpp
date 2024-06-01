#include "HostBase.hpp"
#include <checksum.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
  #define __REV16(number) (__builtin_bswap16(number))
#else
  #define __REV16(number) (((number) >> 8) | ((number) << 8));
#endif

/**
 * @brief 帧同步
 *
 * @param head 接收到的帧头
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostBase::sync(Header& head)
{
    while (!_buf_head.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf_head.push(byte);
    }

    head = _buf_head.get();
    return true;
}

/**
 * @brief 接收数据帧
 *
 * @param cmd [out]接收到的命令
 * @param err [out]错误码
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostBase::recv(Command& cmd, ErrorCode& err, Extra& extra)
{
Start:
    Header head;
    if (!sync(head)) return false;

    cmd = REMOVE_ENCRYPT_MARK(head.cmd);
    err = head.error;
    extra.reset();
    extra.size()      = std::min(head.size, extra.capacity());
    extra.encrypted() = IS_ENCRYPTED(head.cmd) && extra.size() > 0;

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
    if (head.address != address) goto Start;

    return true;
}

/**
 * @brief 接收响应
 *
 * @param cmd 期望的命令
 * @param err 错误码
 * @param extra 附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostBase::recv_response(const Command cmd, ErrorCode& err, Extra& extra)
{
Start:
    Command r_cmd;
    if (!recv(r_cmd, err, extra)) return false;

    // 验证命令
    if (r_cmd != cmd) goto Start;
    return true;
}

/**
 * @brief 发送数据帧
 *
 * @param head 帧头
 * @param extra 附加参数
 * @param size 附加参数长度
 */
void HostBase::send(const Header& head, const void* extra, const uint16_t size)
{
    Chksum chksum = crc_ccitt_ffff((uint8_t*)&head, sizeof(head));
    // 注意: CRC-16 校验和大小端翻转后, 在接收端计算时才会为 0
    chksum        = __REV16(chksum);
    tx(&head, sizeof(head));
    tx(&chksum, sizeof(chksum));

    // 数据为空, 跳过发送
    if (size == 0) return;

    // 发送额外参数
    tx(extra, size);
    // 计算并发送数据校验和
    chksum = crc_ccitt_ffff((uint8_t*)extra, size);
    // 注意: CRC-16 校验和大小端翻转后, 在接收端计算时才会为 0
    chksum = __REV16(chksum);
    tx(&chksum, sizeof(chksum));
}

/**
 * @brief 发送数据帧
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @param encrypt 是否加密?
 * @param err 错误码
 */
void HostBase::send(const Command cmd, Extra& extra, const bool encrypt, const ErrorCode err)
{
    // 截断多余的数据
    extra.truncate();
    if (encrypt) extra.encrypt(secret.nonce, secret.key);
    Header head;
    head.address = address;
    head.cmd     = extra.encrypted() ? ADD_ENCRYPT_MARK(cmd) : REMOVE_ENCRYPT_MARK(cmd);
    head.error   = err;
    head.size    = extra.size();
    if (extra.encrypted())
        send(head, extra.tag(), extra.size() + sizeof(TagType));
    else
        send(head, extra.data(), extra.size());
}

/**
 * @brief 发送 ACK 响应
 *
 */
void HostBase::send_ack()
{
    Header head;
    head.address = address;
    head.cmd     = Command::ACK;
    head.error   = ErrorCode::S_OK;
    head.size    = 0;
    send(head, nullptr, 0);
}

/**
 * @brief 接收 ACK 响应
 *
 * @return true 成功接收 ACK
 * @return false 接收超时
 */
bool HostBase::recv_ack()
{
Start:
    Header head;
    if (!sync(head)) return false;

    // 验证命令
    if (REMOVE_ENCRYPT_MARK(head.cmd) != Command::ACK) goto Start;

    // 验证地址
    if (head.address != address) goto Start;

    return true;
}