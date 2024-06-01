#include "HostBase.hpp"
#include <checksum.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
  #define __REV16(number) (__builtin_bswap16(number))
#else
  #define __REV16(number) (((number) >> 8) | ((number) << 8));
#endif

/**
 * @brief 编码数据帧
 *
 * @param head 帧头
 * @param h_size 帧头长度
 * @param extra 附加参数
 * @param size 附加参数长度
 * @param tx 数据接收方法
 */
void HostBase::_encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint16_t size)
{
    // 写入帧头校验和
    uint16_t* h_chksum = (uint16_t*)(head + h_size - sizeof(Chksum));
    *h_chksum          = crc_ccitt_ffff(head, h_size - sizeof(Chksum));
    // 注意: CRC-16 校验和大小端翻转后, 在接收端计算时才会为 0
    *h_chksum          = __REV16(*h_chksum);
    tx(head, h_size);

    // 数据为空, 跳过发送
    if (size == 0) return;

    // 发送额外参数
    tx(extra, size);
    // 计算并发送数据校验和
    Chksum chksum = crc_ccitt_ffff(extra, size);
    // 注意: CRC-16 校验和大小端翻转后, 在接收端计算时才会为 0
    chksum        = __REV16(chksum);
    tx((uint8_t*)&chksum, sizeof(chksum));
}

/**
 * @brief 发送请求
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 */
void HostBase::send_request(const Command cmd, Extra& extra, const bool encrypt)
{
    if (encrypt) extra.encrypt(secret.nonce, secret.key);
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
 * @brief 接收请求
 *
 * @param cmd [out]接收到的命令
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostBase::recv_request(Command& cmd, Extra& extra)
{
Start:
    // 帧同步
    while (!_buf_req.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf_req.push(byte);
    }

    Request req = _buf_req.get();
    cmd         = req.cmd;
    extra.reset();
    extra.size()      = std::min(req.size, extra.capacity());
    extra.encrypted() = IS_ENCRYPTED(req.cmd) && extra.size() > 0;

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
    if (req.address != address) goto Start;

    return true;
}

/**
 * @brief 发送响应帧
 *
 * @param cmd 响应的指令
 * @param err 错误码
 * @param extra 附加参数
 */
void HostBase::send_response(const Command cmd, const ErrorCode err, Extra& extra)
{
    // 截断多余的数据
    extra.truncate();
    // 若有加密标记, 则对参数进行加密
    if (IS_ENCRYPTED(cmd)) extra.encrypt(secret.nonce, secret.key);
    Response rep;
    rep.address = address;
    rep.cmd     = extra.encrypted() ? ADD_ENCRYPT_MARK(cmd) : REMOVE_ENCRYPT_MARK(cmd);
    rep.error   = err;
    rep.size    = extra.size();
    if (extra.encrypted())
        _encode((uint8_t*)&rep, sizeof(rep), extra.tag(), extra.size() + sizeof(TagType));
    else
        _encode((uint8_t*)&rep, sizeof(rep), extra.data(), extra.size());
}

/**
 * @brief 接收响应
 *
 * @param cmd [out]期望响应的命令
 * @param extra [out]附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostBase::recv_response(const Command cmd, ErrorCode& err, Extra& extra)
{
Start:
    // 帧同步
    while (!_buf_rep.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf_rep.push(byte);
    }

    Response rep = _buf_rep.get();
    err          = rep.error;
    extra.reset();
    extra.size()      = std::min(rep.size, extra.capacity());
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

    // 验证命令
    if (REMOVE_ENCRYPT_MARK(rep.cmd) != cmd)
    {
        // 输出 log 信息
        if (REMOVE_ENCRYPT_MARK(rep.cmd) == Command::LOG)
        {
            log_output((const char*)extra.data(), extra.remain());
        }
        goto Start;
    }

    return true;
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
    // 帧同步
    while (!_buf_rep.verify())
    {
        uint8_t byte;
        if (!rx(byte)) return false; // 接收超时
        _buf_rep.push(byte);
    }

    Response rep = _buf_rep.get();

    // 验证地址
    if (rep.address != address) goto Start;

    // 验证命令
    if (REMOVE_ENCRYPT_MARK(rep.cmd) != Command::ACK) goto Start;

    return true;
}