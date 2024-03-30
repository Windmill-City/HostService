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
    uint8_t*  extra;
    ErrorCode err;
    uint8_t   size;
    if (!recv_response(cmd, err, &extra, size)) return false;
    return true;
}

/**
 * @brief 发送请求
 *
 * @param cmd 请求的指令
 * @param extra 附加参数
 * @param size 参数长度
 */
void HostClient::send_request(const Command cmd, const uint8_t* extra, const uint8_t size)
{
    Request req;
    req.address = address;
    req.cmd     = cmd;
    req.size    = size;
    _encode((uint8_t*)&req, sizeof(req), extra, size);
}

/**
 * @brief 接收响应
 *
 * @param cmd [out]响应的命令
 * @param extra [out]附加参数数组的指针, 调用者无需释放此指针
 * @param size [out]参数长度
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 *
 * 注意: 接收响应和接收请求共用一个缓冲区
 */
bool HostClient::recv_response(Command& cmd, ErrorCode& err, uint8_t** extra, uint8_t& size)
{
    if (extra) *extra = _extra;
    size = 0;
    return _decode_rep(cmd, err, *extra, size);
}

/**
 * @brief 解码响应帧
 *
 * @param cmd 响应的指令
 * @param err 错误码
 * @param extra 附加参数
 * @param size 附加参数长度
 * @param rx 数据接收方法
 * @return true 成功接收一帧
 * @return false 没有接收一帧
 */
bool HostClient::_decode_rep(Command& cmd, ErrorCode& err, uint8_t* extra, uint8_t& size)
{
    // 解码帧头
    if (!_decode_head((uint8_t*)&_rep, sizeof(_rep))) return false;

    cmd  = _rep.cmd;
    size = _rep.size;
    err  = _rep.error;

    // 数据为空, 跳过接收
    if (size == 0) return true;

    // 读取数据
    for (size_t i = 0; i < size + sizeof(Chksum); i++)
    {
        extra[i] = rx();
    }
    // 验证数据
    if (crc_ccitt_ffff(extra, size + sizeof(Chksum)) != 0) return false;

    return true;
}