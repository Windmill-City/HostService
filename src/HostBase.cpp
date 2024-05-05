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
void HostBase::_encode(uint8_t* head, const uint8_t h_size, const uint8_t* extra, const uint8_t size)
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
