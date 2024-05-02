#pragma once
#include "Property.hpp"
#include "Struct.hpp"
#include <algorithm>

enum class RangeMode : uint8_t
{
    Hard,  // 超出范围的赋值 不会 生效
    Soft,  // 超出范围的赋值 会 生效
    Clamp, // 超出范围的赋值会被截断到范围以内
};

/**
 * @brief 存放范围属性的结构体
 *
 * @tparam T 属性的类型
 */
template <typename T>
struct _Range
{
    T min;
    T max;
};

/**
 * @brief 创建范围属性值
 *
 * 属性值的读写是线程安全的
 *
 * 注意: 你不能在中断函数中读写属性值
 *
 * 设置命令:
 * Min,Max(GetSize返回的是写入命令的长度)
 * 读取命令:
 * Min,Max,AbsMin,AbsMax
 *
 * @tparam T 数值类型
 * @tparam AbsMin 绝对最小值
 * @tparam AbsMax 绝对最大值
 * @tparam access 访问级别
 */
template <Number T, T AbsMin, T AbsMax, Access access = Access::READ_WRITE>
struct Range : public Struct<_Range<T>, access>
{
    Range(T min = AbsMin, T max = AbsMax)
    {
        this->min() = AbsMin;
        this->max() = AbsMax;
        safe_set({min, max});
    }

    /**
     * @brief 获取最小值的引用
     *
     * 注意: 访问引用需要加锁
     *
     * @return T& 最小值的引用
     */
    T& min()
    {
        return this->_value.min;
    }

    /**
     * @brief 获取最大值的引用
     *
     * 注意: 访问引用需要加锁
     *
     * @return T& 最大值的引用
     */
    T& max()
    {
        return this->_value.max;
    }

    /**
     * @brief 检查数值是否在范围内
     *
     * 注意: 此方法线程安全
     *
     * @param value 要检查的数值
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool in_range(T value)
    {
        return value >= min() && value <= max();
    }

    virtual ErrorCode safe_set(const _Range<T> value) override
    {
        if (value.min < AbsMin) return ErrorCode::E_INVALID_ARG;
        if (value.max > AbsMax) return ErrorCode::E_INVALID_ARG;
        if (value.min > value.max) return ErrorCode::E_INVALID_ARG;

        this->_value = value;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        extra.add(this->safe_get());
        extra.add(AbsMin);
        extra.add(AbsMax);
        return ErrorCode::S_OK;
    }
};

/**
 * @brief 带范围限制的属性值
 *
 * 属性值的读写是线程安全的
 *
 * @tparam T 属性值类型
 * @tparam AbsMin 绝对最小值
 * @tparam AbsMax 绝对最大值
 * @tparam mode 限制模式
 * @tparam range 范围的访问级
 * @tparam val 属性值的访问级
 */
template <typename T,
          int       AbsMin,
          int       AbsMax,
          RangeMode mode  = RangeMode::Hard,
          Access    range = Access::READ_WRITE,
          Access    val   = Access::READ_WRITE>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
struct RangedProperty : public Property<T, val>
{
    Range<T, AbsMin, AbsMax, range> _range;

    explicit RangedProperty(T value = (AbsMin + AbsMax) / 2, T min = AbsMin, T max = AbsMax)
    {
        _range = {min, max};
        safe_set((AbsMax + AbsMin) / 2); // 默认属性值初始化为中间值
        safe_set(value);                 // 此处赋值可能失败
    }

    /**
     * @brief 获取最小值的引用
     *
     * 注意: 访问引用需要加锁
     *
     * @return T& 最小值的引用
     */
    T& min()
    {
        return _range.min();
    }

    /**
     * @brief 获取最大值的引用
     *
     * 注意: 访问引用需要加锁
     *
     * @return T& 最大值的引用
     */
    T& max()
    {
        return _range.max();
    }

    /**
     * @brief 检查数值是否在范围内
     *
     * 注意: 此方法线程安全
     *
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool in_range()
    {
        return _range.in_range(this->_value);
    }

    /**
     * @brief 将数值限定到范围内
     *
     * 注意: 此方法线程安全
     *
     * @return 自身的引用
     */
    auto& clamp()
    {
        this->_value = std::clamp(this->_value, min(), max());
        return *this;
    }

    /**
     * @brief 设置属性值
     *
     * 支持不同类型的数值赋值
     *
     * 注意: 此方法线程安全
     *
     * @tparam K 数值类型
     * @param other 要设置的属性值
     * @return auto& 自身的引用
     */
    template <Number K>
    auto& operator=(const K other)
    {
        safe_set(other);
        return *this;
    }

    /**
     * @brief 设置属性值
     *
     * 支持不同类型的RangedProperty的赋值
     *
     * 注意: 此方法线程安全
     *
     * @tparam K 数值类型
     * @param other 要设置的属性值
     * @return auto& 自身的引用
     */
    template <Number K>
    auto& operator=(const Property<K>& other)
    {
        safe_set(other);
        return *this;
    }

    virtual ErrorCode safe_set(T value) override
    {
        if (mode == RangeMode::Soft)
        {
            this->_value = value;
            return ErrorCode::S_OK;
        }

        if (mode == RangeMode::Clamp)
        {
            // 限定到范围内
            this->_value = std::clamp(value, min(), max());
            return ErrorCode::S_OK;
        }

        if (mode == RangeMode::Hard)
        {
            // 阻止超范围的赋值
            if (value < min()) return ErrorCode::E_OVER_LOW_LIMIT;
            if (value > max()) return ErrorCode::E_OVER_HIGH_LIMIT;

            this->_value = value;
            return ErrorCode::S_OK;
        }

        return ErrorCode::E_NO_IMPLEMENT;
    }
};