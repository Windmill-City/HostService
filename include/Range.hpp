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

template <typename T>
struct _Range
{
    T min;
    T max;
};

template <Number T, T AbsMin, T AbsMax, Access access = Access::READ_WRITE>
struct Range : public Struct<_Range<T>, access>
{
    Range(T min = AbsMin, T max = AbsMax)
    {
        this->get().min = AbsMin;
        this->get().max = AbsMax;
        _assign({min, max});
    }

    virtual ErrorCode _assign(const _Range<T>& value) override
    {
        if (value.min < AbsMin) return ErrorCode::E_INVALID_ARG;
        if (value.max > AbsMax) return ErrorCode::E_INVALID_ARG;
        if (value.min > value.max) return ErrorCode::E_INVALID_ARG;

        this->_value = value;
        return ErrorCode::S_OK;
    }

    /* 赋值运算符 */
    template <typename K>
    auto& operator=(const _Range<K> other)
    {
        _assign(other);
        return *this;
    }
};

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

    explicit RangedProperty(T value = 0, T min = AbsMin, T max = AbsMax)
    {
        _range       = {min, max};
        this->_value = value;

        if (mode == RangeMode::Hard || mode == RangeMode::Clamp) this->clamp();
    }

    /**
     * @brief 获取最小值的引用
     *
     * @return T& 最小值的引用
     */
    T min()
    {
        return _range.get().min;
    }

    /**
     * @brief 获取最大值的引用
     *
     * @return T& 最大值的引用
     */
    T max()
    {
        return _range.get().max;
    }

    /**
     * @brief 检查数值是否在范围内
     *
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool in_range()
    {
        return this->_value >= min() && this->_value <= max();
    }

    /**
     * @brief 将数值限定到范围内
     *
     * @return 自身引用
     */
    auto& clamp()
    {
        this->_value = std::clamp(this->_value, min(), max());
        return *this;
    }

    virtual ErrorCode _assign(T value) override
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

        return ErrorCode::S_OK;
    }

    /* 赋值运算符 */
    template <typename K>
    auto& operator=(const K other)
    {
        _assign(other);
        return *this;
    }
};