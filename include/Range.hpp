#pragma once
#include "Property.hpp"
#include <algorithm>

enum class RangeMode : uint8_t
{
    Hard,  // 超出范围的赋值 不会 生效
    Soft,  // 超出范围的赋值 会 生效
    Clamp, // 超出范围的赋值会被截断到范围以内
};

enum class RangeAccess : uint8_t
{
    Range = 0, // 范围值
    Absolute   // 绝对范围
};

template <typename T>
concept Number = std::is_arithmetic_v<T>;

/**
 * @brief 存放范围属性的结构体
 *
 * @tparam T 属性的类型
 */
template <Number T>
struct RangeVal
{
    T min;
    T max;
};

/**
 * @brief 范围属性值模板
 *
 * 设置命令:
 * Range,Min,Max
 * 读取命令:
 * Range,Min,Max
 * Absolute,Min,Max
 *
 * @tparam T 数值类型
 * @tparam AbsMin 绝对最小值
 * @tparam AbsMax 绝对最大值
 * @tparam access 访问级别
 */
template <Number T, T AbsMin, T AbsMax, Access access = Access::READ_WRITE>
struct Range : public Property<RangeVal<T>, access>
{
    explicit Range(T min = AbsMin, T max = AbsMax)
    {
        this->min() = AbsMin;
        this->max() = AbsMax;
        safe_set({min, max});
    }

    /**
     * @brief 获取最小值的引用
     *
     * @note 此方法非线程安全
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
     * @note 此方法非线程安全
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
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @param value 要检查的数值
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool in_range(T value)
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        return value >= min() && value <= max();
    }

    virtual ErrorCode safe_set(const RangeVal<T> value) override
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        if (value.min < AbsMin) return ErrorCode::E_INVALID_ARG;
        if (value.max > AbsMax) return ErrorCode::E_INVALID_ARG;
        if (value.min > value.max) return ErrorCode::E_INVALID_ARG;

        this->_value = value;
        return ErrorCode::S_OK;
    }

    /**
     * @brief 设置属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @tparam K 数值类型
     * @param other 要设置的属性值
     * @return auto& 自身的引用
     */
    auto& operator=(const RangeVal<T> other)
    {
        safe_set(other);
        return *this;
    }

    virtual ErrorCode set(Extra& extra) override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;

        switch (access)
        {
        case RangeAccess::Range:
            RangeVal<T> value;
            if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;

            safe_set(value);
            return ErrorCode::S_OK;
        case RangeAccess::Absolute:
            return ErrorCode::E_READ_ONLY;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;

        switch (access)
        {
        case RangeAccess::Range:
            extra.add(this->safe_get());
            return ErrorCode::S_OK;
        case RangeAccess::Absolute:
            extra.add<T>(AbsMin);
            extra.add<T>(AbsMax);
            return ErrorCode::S_OK;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;
        switch (access)
        {
        case RangeAccess::Range:
        case RangeAccess::Absolute:
            extra.add<uint16_t>(2 * sizeof(T));
            return ErrorCode::S_OK;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }
};

/**
 * @brief 带范围限制的属性值模板
 *
 * @tparam T 属性值类型
 * @tparam mode 限制模式
 * @tparam access 属性值的访问级
 */
template <Number T, RangeMode mode = RangeMode::Hard, Access access = Access::READ_WRITE>
struct RangedProperty : public Property<T, access>
{
    RangedProperty(const RangeVal<T>& range, T val = 0)
        : _range(range)
    {
        this->_value = val;
        clamp();
    }

    /**
     * @brief 获取最小值
     *
     * @note 此方法线程安全
     *
     * @return T 最小值
     */
    T min()
    {
        return _range.min;
    }

    /**
     * @brief 获取最大值
     *
     * @note 此方法线程安全
     *
     * @return T 最大值
     */
    T max()
    {
        return _range.max;
    }

    /**
     * @brief 检查数值是否在范围内
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return true 在范围内
     * @return false 不在范围内
     */
    bool in_range()
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        return this->_value >= min() && this->_value <= max();
    }

    /**
     * @brief 将数值限定到范围内
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return 自身的引用
     */
    auto& clamp()
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        this->_value = std::clamp(this->_value, min(), max());
        return *this;
    }

    /**
     * @brief 设置属性值
     *
     * 支持不同类型的数值赋值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
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
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
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
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
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

  protected:
    const RangeVal<T>& _range;
};