#pragma once
#include "HostBase.hpp"
#include <array>

struct Extra
{
    enum class Type
    {
        RAW = 0,
        ID_ONLY,
        ID_AND_MEMORY,
    };

#pragma pack(1)

    struct _Memory
    {
        uint16_t offset;
        uint8_t  datlen;
        uint8_t  data[];
    };

    struct _Extra
    {
        uint16_t id;

        union
        {
            _Memory mem;
            uint8_t data[];
        } uni;
    };

#pragma pack()

    // 缓冲区长度
    uint8_t                                         _tail;
    // 缓冲区数据索引
    uint8_t                                         _data;
    // 附加参数缓冲区
    std::array<uint8_t, UINT8_MAX + sizeof(Chksum)> _buf;
    // 附加参数指针
    _Extra*                                         _extra;

    Extra(Type _type = Type::ID_ONLY)
    {
        switch (_type)
        {
        case Type::RAW:
            _tail = _data = 0;
            break;
        case Type::ID_ONLY:
            _tail = _data = sizeof(_Extra::id);
            break;
        case Type::ID_AND_MEMORY:
            _tail = _data = sizeof(_Extra::id) + sizeof(_Memory);
            break;
        }
        _extra = (_Extra*)_buf.data();
    }

    /**
     * @brief 获取属性Id的引用
     *
     * @return uint16_t 属性Id
     */
    uint16_t& id()
    {
        return _extra->id;
    }

    /**
     * @brief 获取内存偏移的引用
     *
     * @return uint16_t& 偏移的引用
     */
    uint16_t& offset()
    {
        return _extra->uni.mem.offset;
    }

    /**
     * @brief 获取内存数据长度的引用
     *
     * @return uint16_t& 长度的引用
     */
    uint8_t& datlen()
    {
        return _extra->uni.mem.datlen;
    }

    /**
     * @brief 添加数据
     *
     * @tparam T 数据类型
     * @param value 数据值
     * @return true 添加成功
     * @return false 附加参数长度超出最大帧长限制
     */
    template <typename T>
    bool add(const T value)
    {
        // 只允许标准布局类型和非指针类型
        static_assert(std::is_standard_layout_v<T> && !std::is_pointer_v<T>);

        if (sizeof(value) + _tail > UINT8_MAX) return false;

        memcpy(&_buf[_tail], (uint8_t*)&value, sizeof(value));
        _tail += sizeof(value);
        return true;
    }

    /**
     * @brief 添加数组类型的数据
     *
     * @tparam T 数组类型
     * @param value 数组指针
     * @param size 数组字节长度
     * @return true 添加成功
     * @return false 附加参数长度超过最大帧长限制
     */
    template <typename T>
    bool add(const T* value, const uint8_t size)
    {
        // 只允许标准布局类型和非指针类型
        static_assert(std::is_standard_layout_v<T> && !std::is_pointer_v<T>);

        if (size + _tail > UINT8_MAX) return false;
        memcpy(&_buf[_tail], (uint8_t*)value, size);
        _tail += size;
        return true;
    }

    /* 指针运算符 */
    uint8_t* operator&()
    {
        return _buf.data();
    }

    /* 数组运算符 */
    uint8_t& operator[](std::size_t idx)
    {
        return _buf[_data + idx];
    }

    /**
     * @brief 获取缓冲区大小的引用
     *
     * @return uint8_t 缓冲区大小的引用
     */
    uint8_t& size()
    {
        return _tail;
    }

    /**
     * @brief 获取指向数据区的指针
     *
     * @return uint8_t* 数据区指针
     */
    uint8_t* data()
    {
        return &_buf[_data];
    }

    /**
     * @brief 获取数据区长度
     *
     * @return uint8_t 数据区长度
     */
    uint8_t data_size()
    {
        return _tail - _data;
    }

    /**
     * @brief 获取缓冲区剩余长度
     *
     * @return uint8_t 剩余长度
     */
    uint8_t remain()
    {
        return UINT8_MAX - _tail;
    }

    /**
     * @brief 丢弃数据区内容
     *
     * @return auto& 自身引用
     */
    auto& truncate()
    {
        _tail = _data;
        return *this;
    }

    /**
     * @brief 尝试从缓冲区中解码出id
     *
     * @return true 成功解码
     * @return false 缓冲区长度不足
     */
    bool decode_id()
    {
        // 检查长度是否过短
        if (data_size() < sizeof(id()))
        {
            return false;
        }
        // 指定数据区指针索引
        _data += sizeof(id());
        return true;
    }

    /**
     * @brief 尝试从缓冲区中解码出内存参数
     *
     * @return true 成功解码
     * @return false 缓冲区长度不足
     */
    bool decode_mem()
    {
        // 检查长度是否过短
        if (data_size() < sizeof(Extra::_Memory)) return false;
        // 指定数据区指针索引
        _data += sizeof(Extra::_Memory);
        return true;
    }

    /**
     * @brief 复位缓冲区
     *
     */
    void reset()
    {
        _tail = _data = 0;
    }
};