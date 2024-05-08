#pragma once
#include <array>
#include <checksum.h>
#include <cstddef>
#include <stdint.h>

enum PopAction
{
    NoPop,     // 队列满时丢弃放入的数据
    PopOnPush, // 队列满时丢弃首部的数据
};

template <size_t _size, PopAction action = PopAction::NoPop>
struct FixedQueue
{
    /**
     * @brief 复位队列
     *
     */
    void reset()
    {
        _empty = 0;
        _data  = 0;
    }

    /**
     * @brief 在队列尾部放入一个元素
     *
     * @param item 要放入的元素
     * @return true 成功
     * @return false 队列满
     */
    bool push(uint8_t item)
    {
        if (full() && action == PopOnPush) pop();
        if (!full())
        {
            _buf[_empty] = item;
            _empty       = (_empty + 1) % (_size + 1);
            return true;
        }
        else
            return false;
    }

    /**
     * @brief 从队列首部弹出一个元素
     *
     * @param item 弹出的元素
     * @return true 成功
     * @return false 队列空
     */
    bool pop(uint8_t* item = nullptr)
    {
        if (!empty())
        {
            if (item) *item = _buf[_data];
            _data = (_data + 1) % (_size + 1);
            return true;
        }
        else
            return false;
    }

    /**
     * @brief 以数组下标的形式访问元素
     *
     * @param idx 下标
     * @return uint8_t 元素
     */
    uint8_t& operator[](size_t idx)
    {
        idx = (idx + _data) % (_size + 1);
        return _buf[idx];
    }

    /**
     * @brief 判断队列是否满
     *
     * @return true 满
     * @return false 未满
     */
    bool full()
    {
        size_t next = (_empty + 1) % (_size + 1);
        return next == _data;
    }

    /**
     * @brief 判断队列是否为空
     *
     * @return true 空
     * @return false 非空
     */
    bool empty()
    {
        return _empty == _data;
    }

    /**
     * @brief 队列元素数量
     *
     * @return size_t 元素数量
     */
    size_t size()
    {
        if (_data <= _empty)
            return _empty - _data;
        else
            return (_size + 1) - (_data - _empty);
    }

    /**
     * @brief 获取队列容量
     *
     * @return size_t 队列容量
     */
    size_t capacity()
    {
        return _size;
    }

  protected:
    // 需要额外占用一个元素来表示队列满
    std::array<uint8_t, (_size + 1)> _buf;
    // 指向空数据
    volatile size_t                  _empty;
    // 指向有效数据
    volatile size_t                  _data;
};
