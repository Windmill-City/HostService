#pragma once
#include "Property.hpp"
#include "Struct.hpp"

enum class RangeMode : uint8_t
{
    Hard,  // 超出范围的赋值 不会 生效
    Soft,  // 超出范围的数值 会 生效
    Clamp, // 超出范围的数值会被截断到范围以内
};

template <typename T>
struct Range : public Property<T>
{
    RangeMode mode = RangeMode::Hard;
    T         min;
    T         max;

    /**
     * @brief 检查数值是否在范围内
     *
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool      in_range()
    {
        return _value >= min && _value <= max;
    }

    /**
     * @brief 将数值限定到范围内
     *
     * @return 自身引用
     */
    auto& clamp()
    {
        _value = std::clamp(_value, min(), max());
        return *this;
    }

    virtual ErrorCode _assign(T value) override
    {
        if (mode == RangeMode::Soft)
        {
            _value = value;
            return ErrorCode::S_OK;
        }

        if (mode == RangeMode::Clamp)
        {
            // 限定到范围内
            _value = std::clamp(value, min(), max());
            return ErrorCode::S_OK;
        }

        if (mode == RangeMode::Hard)
        {
            // 阻止超范围的赋值
            if (value < min()) return ErrorCode::E_OVER_LOW_LIMIT;
            if (value > max()) return ErrorCode::E_OVER_HIGH_LIMIT;

            _value = value;
            return ErrorCode::S_OK;
        }
    }

    virtual ErrorCode get(uint8_t** p_value, uint8_t& size) override
    {
        size    = sizeof(_value);
        p_value = &_value;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(const uint8_t* p_value, const uint8_t size) override
    {
        if (size != sizeof(_value))
        {
            return ErrorCode::E_INVALID_ARG;
        }

        _assign(*(T*)p_value);
        return ErrorCode::S_OK;
    }
};