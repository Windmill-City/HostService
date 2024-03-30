#pragma once
#include "Property.hpp"
#include "Struct.hpp"
#include <algorithm>

enum class RangeMode : uint8_t
{
    Hard,  // 超出范围的赋值 不会 生效
    Soft,  // 超出范围的数值 会 生效
    Clamp, // 超出范围的数值会被截断到范围以内
};

template <typename T>
struct Range : public Property<T>
{
    struct _Range
    {
        T min;
        T max;
    };

    RangeMode      mode = RangeMode::Hard;
    Struct<_Range> _range;

    Range()
        : Range(0, 0, 0)
    {
    }

    Range(T value, T min, T max)
        : Range(value, min, max, Access::READ)
    {
    }

    Range(T value, T min, T max, Access access)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);

        this->_value = value;
        this->access = access;
        this->min()  = min;
        this->max()  = max;
        this->clamp();
    }

    /**
     * @brief 获取最小值的引用
     *
     * @return T& 最小值的引用
     */
    T& min()
    {
        return _range.get().min;
    }

    /**
     * @brief 获取最大值的引用
     *
     * @return T& 最大值的引用
     */
    T& max()
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
};