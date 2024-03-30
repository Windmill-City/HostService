#pragma once
#include "HostClient.hpp"

struct RequestBuilder
{
    RequestBuilder()
    {
        // 最大附加数据长度为255
        _buf.reserve(UINT8_MAX);
    }

    /**
     * @brief 添加属性Id
     *
     * @param id 属性Id
     * @return true 添加成功
     * @return false 附加参数长度超出最大帧长限制
     */
    bool id(const uint16_t id)
    {
        return add(id);
    }

    /**
     * @brief 添加附加参数
     *
     * @tparam T 参数类型
     * @param value 参数值
     * @return true 添加成功
     * @return false 附加参数长度超出最大帧长限制
     */
    template <typename T>
    bool add(const T value)
    {
        // 只允许标准布局类型和非指针类型
        static_assert(std::is_standard_layout_v<T> && !std::is_pointer_v<T>);

        if (sizeof(value) + _buf.size() > 255) return false;

        for (size_t i = 0; i < sizeof(value); i++)
        {
            _buf.push_back(((uint8_t*)&value)[i]);
        }

        return true;
    }

    /**
     * @brief 添加数组类型的附加参数
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

        if (size + _buf.size() > 255) return false;

        for (size_t i = 0; i < size; i++)
        {
            _buf.push_back(((uint8_t*)value)[i]);
        }

        return true;
    }

    /**
     * @brief 发送请求
     *
     * @param hs HostClient 客户端实例
     * @param cmd 请求的命令
     */
    void tx(HostClient& hs, Command cmd)
    {
        hs.send_request(cmd, _buf.data(), _buf.size());
    }

    // 附加参数缓冲区
    std::vector<uint8_t> _buf;
};