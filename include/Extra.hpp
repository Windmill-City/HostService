#pragma once
#include <string.h>
#include <uaes.h>

#include <Types.hpp>

template <size_t _size>
struct ExtraT
{
  public:
    /**
     * @brief 向缓冲区中添加指定类型的数据
     *
     * @tparam T 数据类型
     * @param value 数据值
     * @return true 添加成功
     * @return false 缓冲区长度不足
     */
    template <PropertyVal T>
    bool add(const T value)
    {
        return add(&value, sizeof(value));
    }

    /**
     * @brief 向缓冲区中添加一个数组
     *
     * @param value 数组指针
     * @param size 数组字节长度
     * @return true 添加成功
     * @return false 缓冲区长度不足
     */
    bool add(const void* value, size_t size)
    {
        // 检查是否超长
        if (size > spare()) return false;
        // 复制数据
        memcpy(curr(), (uint8_t*)value, size);
        seek(_data + size);
        return true;
    }

    /**
     * @brief 从缓冲区中获取指定类型的数据
     *
     * @tparam T 数据类型
     * @param value 接收数据的缓冲区引用
     * @return true 成功获取
     * @return false 缓冲区长度不足
     */
    template <PropertyVal T>
    bool get(T& value)
    {
        return get(&value, sizeof(value));
    }

    /**
     * @brief 从缓冲区中获取一个数组
     *
     * @param value 接收数据的缓冲区
     * @param size 数组字节长度
     * @return true 成功获取
     * @return false 缓冲区长度不足
     */
    bool get(void* value, size_t size)
    {
        // 检查长度是否足够
        if (size > remain()) return false;
        // 拷贝数据
        memcpy(value, curr(), size);
        // 更新数据指针
        _data += size;
        return true;
    }

    /**
     * @brief 解密缓冲区数据
     *
     * @param nonce 加密数据的随机数
     * @param key 加密数据的密钥
     * @return true 解密成功
     * @return false 解密失败
     */
    bool decrypt(const NonceType& nonce, const KeyType& key)
    {
        // 不能重复解密
        if (!encrypted()) return false;
        encrypted() = false;
        // 数据长度至少为 1
        if (size() == 0) return false;

        if (!UAES_CCM_SimpleDecrypt(key.data(),
                                    key.size(),
                                    nonce.data(),
                                    nonce.size(),
                                    NULL,
                                    0,
                                    data(),
                                    data(),
                                    size(),
                                    tag(),
                                    sizeof(TagType)))
        {
            reset();
            return false;
        }
        return true;
    }

    /**
     * @brief 加密缓冲区数据
     *
     * @param nonce 加密数据的随机数
     * @param key 加密数据的密钥
     */
    void encrypt(const NonceType& nonce, const KeyType& key)
    {
        // 数据长度至少为 1
        if (size() == 0) return;
        // 不能重复加密
        if (encrypted()) return;
        encrypted() = true;

        UAES_CCM_SimpleEncrypt(key.data(),
                               key.size(),
                               nonce.data(),
                               nonce.size(),
                               NULL,
                               0,
                               data(),
                               data(),
                               size(),
                               tag(),
                               sizeof(TagType));
    }

    /**
     * @brief 返回缓冲区是否加密的引用
     *
     * @return true 已加密
     * @return false 未加密
     */
    bool& encrypted()
    {
        return _encrypted;
    }

    /**
     * @brief 设置当前数据指针
     *
     * @param offset 相对缓冲区基地址的偏移
     */
    void seek(Size offset)
    {
        _data = offset;
        if (_tail < _data) _tail = _data;
    }

    /**
     * @brief 返回 tag的首地址
     *
     * @return uint8_t* tag的首地址
     */
    uint8_t* tag()
    {
        return _tag.data();
    }

    /**
     * @brief 返回数据区的首地址
     *
     * @return uint8_t* 数据区的首地址
     */
    uint8_t* data()
    {
        return _buf.data();
    }

    /**
     * @brief 返回当前数据的指针
     *
     * @return uint8_t* 当前数据指针
     */
    uint8_t* curr()
    {
        return &_buf[_data];
    }

    /**
     * @brief 返回未读数据的长度
     *
     * @return Size 未读数据长度
     */
    Size remain() const
    {
        return _tail - _data;
    }

    /**
     * @brief 返回空闲区域的长度
     *
     * @return Size 空闲区域长度
     */
    Size spare() const
    {
        return _size - _tail;
    }

    /**
     * @brief 返回缓冲区总长度
     *
     * @return Size 缓冲区总长度
     */
    Size& size()
    {
        return _tail;
    }

    /**
     * @brief 返回缓冲区最大容量
     *
     * @return Size 最大容量
     */
    Size capacity() const
    {
        return _size;
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
     * @brief 丢弃未读取的内容
     *
     */
    void truncate()
    {
        _tail = _data;
    }

    /**
     * @brief 将所有数据标记为已读取
     *
     */
    void readall()
    {
        seek(size());
    }

  protected:
    // 缓冲区是否加密
    bool                       _encrypted = false;
    // 数据区长度
    Size                       _tail      = 0;
    // 未读数据的偏移
    Size                       _data      = 0;
    // Tag区
    TagType                    _tag;
    // 缓冲区
    std::array<uint8_t, _size> _buf;
};

#ifndef MEMORY_ACCESS_SIZE_MAX
  #define MEMORY_ACCESS_SIZE_MAX 1024
#endif

using Extra = ExtraT<MEMORY_ACCESS_SIZE_MAX + sizeof(PropertyId) + sizeof(MemoryAccess)>;
