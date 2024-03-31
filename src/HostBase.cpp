#include "HostBase.hpp"
#include <checksum.h>
#include <string.h>

#if defined(_MSC_VER)
  #define __REV16(number) (_byteswap_ushort(number))
#elif defined(__GNUC__) || defined(__clang__)
  #define __REV16(number) (__builtin_bswap16(number))
#else
  #error "Unsupported compiler!"
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
void HostBase::_encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint8_t size)
{
    // 写入帧头校验和
    uint16_t* h_chksum = (uint16_t*)(head + h_size - sizeof(Chksum));
    *h_chksum          = crc_ccitt_ffff(head, h_size - sizeof(Chksum));
    // 注意: CRC-16 校验和**必须**进行大小端翻转, 在接收端计算时才会为 0
    *h_chksum          = __REV16(*h_chksum);
    tx(head, h_size);

    // 数据为空, 跳过发送
    if (size == 0) return;

    // 发送额外参数
    tx(extra, size);
    // 计算并发送数据校验和
    Chksum chksum = crc_ccitt_ffff(extra, size);
    // 注意: CRC-16 校验和**必须**进行大小端翻转, 在接收端计算时才会为 0
    chksum        = __REV16(chksum);
    tx((uint8_t*)&chksum, sizeof(chksum));
}

/**
 * @brief 完成帧同步工作
 *
 * @param head 帧头
 * @param h_size 帧头长度
 * @param rx 数据接收方法
 * @return true 接收到帧头
 * @return false 没有接收到帧头
 */
bool HostBase::_decode_head(uint8_t* head, uint8_t h_size)
{
    // 帧同步 - 每次读取 1 字节数据
    // 从队列头部弹出 1 字节, 头部为低地址
    memmove(head, head + 1, h_size - 1);
    // 从队列尾部放入 1 字节
    *(head + h_size - 1) = rx();

    // 验证帧头有效性
    if (crc_ccitt_ffff(head, h_size) != 0) return false;

    return true;
}
