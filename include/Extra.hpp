#pragma once
#include <array>
#include <Common.hpp>
#include <string.h>
#include <uaes.h>

// 只允许标准布局类型和非指针类型
template <typename T>
concept Data    = std::is_standard_layout_v<T> && !std::is_pointer_v<T>;

using TagType   = std::array<uint8_t, 16>;
using NonceType = std::array<uint8_t, 12>;
using KeyType   = std::array<uint8_t, 256 / 8>;

template <size_t _size>
struct ExtraT
{
  public:
    /**
     * @brief 添加数据
     *
     * @tparam T 数据类型
     * @param value 数据值
     * @return true 添加成功
     * @return false 附加参数长度超出最大帧长限制
     */
    template <Data T>
    bool add(const T value)
    {
        // 检查是否超长
        if (sizeof(value) + _data > _size) return false;
        // 复制数据
        memcpy(&_buf[_data], (uint8_t*)&value, sizeof(value));
        seek(_data + sizeof(value));
        return true;
    }

    /**
     * @brief 添加数组
     *
     * @tparam T 数据类型
     * @param value 数组指针
     * @param size 数组字节长度
     * @return true 添加成功
     * @return false 附加参数长度超过最大帧长限制
     */
    template <Data T>
    bool add(const T* value, const size_t size)
    {
        // 检查是否超长
        if (size + _data > _size) return false;
        // 复制数据
        memcpy(&_buf[_data], (uint8_t*)value, size);
        seek(_data + size);
        return true;
    }

    /**
     * @brief 读取数据
     *
     * @tparam T 数据类型
     * @param value 数据的引用
     * @return true 成功读取
     * @return false 数据长度不足
     */
    template <Data T>
    bool get(T& value)
    {
        // 检查长度是否过短
        if (remain() < sizeof(T)) return false;
        value = *(T*)&_buf[_data];
        // 更新数据指针
        _data += sizeof(T);
        return true;
    }

    /**
     * @brief 读取数组
     *
     * @tparam T 数据类型
     * @param value 数组指针
     * @param size 数组字节长度
     * @return true 成功读取
     * @return false 数据长度不足
     */
    template <Data T>
    bool get(T* value, size_t size)
    {
        // 检查长度是否过短
        if (remain() < size) return false;
        // 拷贝数据
        memcpy(value, data(), size);
        // 更新数据指针
        _data += size;
        return true;
    }

    /**
     * @brief 解密缓冲区数据
     *
     *
     * @details
     * 加密数据的结构:
     * [authentication_tag]:数据校验码, 长度: 同AES密钥长度
     * [encrypted_data]: 加密后的数据
     * 解密后数据的结构:
     * [authentication_tag]:数据校验码, 长度: 同AES密钥长度
     * [decrypted_data]:解密后的数据
     *
     * @return true 解密成功
     * @return false 解密失败
     */
    bool decrypt(const NonceType& nonce, const KeyType& key)
    {
        if (!_encrypted) return false;
        _encrypted = false;
        // 数据长度至少为 1
        if (sizeof(TagType) >= size()) return false;
        seek(sizeof(TagType));

        size_t   data_len = remain();
        uint8_t* _data    = &_buf[sizeof(TagType)];
        uint8_t* _tag     = &_buf[0];
        return UAES_CCM_SimpleDecrypt(key.data(),
                                      key.size(),
                                      nonce.data(),
                                      nonce.size(),
                                      NULL,
                                      0,
                                      _data,
                                      _data,
                                      data_len,
                                      _tag,
                                      sizeof(TagType));
    }

    /**
     * @brief 加密缓冲区数据
     *
     * @note 缓冲区前部需要预留数据校验码的空位
     *
     * @param aes AES加密数据
     */
    void encrypt(const NonceType& nonce, const KeyType& key)
    {
        if (_encrypted) return;
        _encrypted = true;
        // 数据长度至少为 1
        if (sizeof(TagType) >= size()) return;
        seek(sizeof(TagType));

        size_t   data_len = remain();
        uint8_t* _data    = &_buf[sizeof(TagType)];
        uint8_t* _tag     = &_buf[0];
        UAES_CCM_SimpleEncrypt(key.data(),
                               key.size(),
                               nonce.data(),
                               nonce.size(),
                               NULL,
                               0,
                               _data,
                               _data,
                               data_len,
                               _tag,
                               sizeof(TagType));
    }

    /**
     * @brief 设置数据区偏移
     *
     * @param offset 指针偏移
     */
    void seek(uint16_t offset)
    {
        _data = offset;
        if (_tail < _data) _tail = _data;
    }

    /**
     * @brief 将所有数据标记为已读取
     *
     */
    void readall()
    {
        seek(size());
    }

    /**
     * @brief 预留校验位的长度
     *
     */
    void reserve_tag()
    {
        seek(sizeof(TagType));
    }

    /**
     * @brief 获取缓冲区的首地址
     *
     * @return uint8_t* 缓冲区的首地址
     */
    uint8_t* operator&()
    {
        return _buf.data();
    }

    /**
     * @brief 以数组下标的方式读写缓冲区的内容
     *
     * @param idx 下标
     * @return uint8_t& 内容
     */
    uint8_t& operator[](size_t idx)
    {
        return _buf[idx];
    }

    /**
     * @brief 获取未读取数据的首指针
     *
     * @return uint8_t* 未读取数据的首指针
     */
    uint8_t* data()
    {
        return &_buf[_data];
    }

    /**
     * @brief 获取未读取数据的长度
     *
     * @return uint8_t 数据长度
     */
    uint16_t remain() const
    {
        return _tail - _data;
    }

    /**
     * @brief 获取缓冲区空闲区域的长度
     *
     * @return uint8_t 空闲区域的长度
     */
    uint16_t spare() const
    {
        return _size - _tail;
    }

    /**
     * @brief 丢弃未读取的内容
     *
     * @return auto& 返回自身的引用
     */
    auto& truncate()
    {
        _tail = _data;
        return *this;
    }

    /**
     * @brief 获取缓冲区长度
     *
     * @return uint8_t 缓冲区长度
     */
    uint16_t& size()
    {
        return _tail;
    }

    /**
     * @brief 复位缓冲区
     *
     */
    void reset()
    {
        _encrypted = false;
        _tail = _data = 0;
    }

    /**
     * @brief 数据区是否加密
     *
     * @return true 已加密
     * @return false 未加密
     */
    bool& encrypted()
    {
        return _encrypted;
    }

    /**
     * @brief 返回缓冲区最大容量
     *
     * @return uint16_t 最大容量
     */
    uint16_t capacity()
    {
        return _size;
    }

  protected:
    // 数据是否加密
    bool                                        _encrypted = false;
    // 缓冲区长度
    uint16_t                                    _tail      = 0;
    // 未读取数据的偏移
    uint16_t                                    _data      = 0;
    // 缓冲区
    std::array<uint8_t, _size + sizeof(Chksum)> _buf;
};

using Extra = ExtraT<UINT8_MAX>;