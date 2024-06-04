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
    Range = 0, // 当前范围
    Absolute   // 范围的绝对最大值
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
    T    min;
    T    max;

    bool operator==(const RangeVal& range) const
    {
        return this->min == range.min && this->max == range.max;
    }
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
 * @tparam access 访问级别
 */
template <Number T, Access access = Access::READ_WRITE>
struct Range : public Property<RangeVal<T>, access>
{
    const RangeVal<T> Absolute;

    explicit Range(const RangeVal<T> abs)
        : Absolute(abs)
    {
        safe_set(Absolute);
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
     * @brief 获取最小值
     *
     * @note 此方法非线程安全
     *
     * @return T 最小值
     */
    const T& min() const
    {
        return this->_value.min;
    }

    /**
     * @brief 获取最大值
     *
     * @note 此方法非线程安全
     *
     * @return T 最大值
     */
    const T& max() const
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
    bool in_range(const T value) const
    {
        LockGuard lock(PropertyBase::MutexGlobal);
        return value >= min() && value <= max();
    }

    virtual ErrorCode safe_set(const RangeVal<T> value) override
    {
        LockGuard lock(PropertyBase::MutexGlobal);
        if (value.min < Absolute.min) return ErrorCode::E_INVALID_ARG;
        if (value.max > Absolute.max) return ErrorCode::E_INVALID_ARG;
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

    virtual ErrorCode set(Extra& extra, bool privileged) override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;

        switch (access)
        {
        case RangeAccess::Range:
        {
            ErrorCode   err;
            RangeVal<T> value;
            if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;
            if ((err = safe_set(value)) != ErrorCode::S_OK) return err;
            extra.reset();
            return ErrorCode::S_OK;
        }
        case RangeAccess::Absolute:
            return ErrorCode::E_READ_ONLY;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }

    virtual ErrorCode get(Extra& extra, bool privileged) const override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;
        extra.reset();

        switch (access)
        {
        case RangeAccess::Range:
            extra.add(this->safe_get());
            return ErrorCode::S_OK;
        case RangeAccess::Absolute:
            extra.add(Absolute);
            return ErrorCode::S_OK;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }

    virtual ErrorCode get_size(Extra& extra, bool privileged) const override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;
        extra.reset();
        switch (access)
        {
        case RangeAccess::Range:
        case RangeAccess::Absolute:
            extra.add<uint16_t>(sizeof(this->_value));
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
    T min() const
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
    T max() const
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
    bool in_range() const
    {
        LockGuard lock(PropertyBase::MutexGlobal);
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
        LockGuard lock(PropertyBase::MutexGlobal);
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

    virtual ErrorCode safe_set(const T value) override
    {
        LockGuard lock(PropertyBase::MutexGlobal);
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