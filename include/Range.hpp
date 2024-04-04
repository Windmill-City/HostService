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
 * @brief 范围属性
 *
 * 设置命令:
 * Min,Max
 * 读取命令:
 * Min,Max,AbsMin,AbsMax(GetSize返回的是读取命令的长度)
 *
 * @tparam T 属性类型
 * @tparam AbsMin 绝对最小值
 * @tparam AbsMax 绝对最大值
 * @tparam access 访问级别
 */
template <Number T, T AbsMin, T AbsMax, Access access = Access::READ_WRITE>
struct Range : public Struct<_Range<T>, access>
{
    Range(T min = AbsMin, T max = AbsMax)
    {
        this->ref().min = AbsMin;
        this->ref().max = AbsMax;
        safe_set({min, max});
    }

    /**
     * @brief 获取最小值
     *
     * @return T& 最小值
     */
    T min()
    {
        return this->safe_get().min;
    }

    /**
     * @brief 获取最大值
     *
     * @return T& 最大值
     */
    T max()
    {
        return this->safe_get().max;
    }

    /**
     * @brief 检查数值是否在范围内
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

    virtual ErrorCode get_size(Extra& extra) override
    {
        // 包含当前范围和绝对最大范围
        extra.add((uint16_t)(sizeof(this->_value) * 2));
        return ErrorCode::S_OK;
    }
};

/**
 * @brief 带范围限制的属性值
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

    RangedProperty(T value = 0, T min = AbsMin, T max = AbsMax)
    {
        _range = {min, max};
        safe_set(value);
    }

    /**
     * @brief 获取最小值
     *
     * @return T& 最小值
     */
    T min()
    {
        return _range.min();
    }

    /**
     * @brief 获取最大值
     *
     * @return T& 最大值
     */
    T max()
    {
        return _range.max();
    }

    /**
     * @brief 检查数值是否在范围内
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
     * @return 自身引用
     */
    auto& clamp()
    {
        safe_set(std::clamp(this->_value, min(), max()));
        return *this;
    }

    /**
     * @brief 设置属性值
     * 
     * 支持不同类型的RangedProperty的赋值
     *
     * @tparam K 参数类型
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