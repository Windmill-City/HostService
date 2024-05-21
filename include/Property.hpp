#pragma once
#include "PropertyBase.hpp"
#include <type_traits>

template <typename T>
concept PropertyVal = std::is_standard_layout_v<T>;

/**
 * @brief 属性值模板
 *
 *
 * @tparam T 数值类型
 * @tparam access 访问级别
 */
template <PropertyVal T, Access access = Access::READ_WRITE>
struct Property : public PropertyAccess<access>
{
    Property(T value = {0})
    {
        safe_set(value);
    }

    /**
     * @brief 读取属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return T 属性值
     */
    virtual T safe_get() const
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        return _value;
    }

    /**
     * @brief 设置属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    virtual ErrorCode safe_set(T value)
    {
#ifndef NO_LOCK
        std::lock_guard lock(PropertyBase::Mutex);
#endif
        _value = value;
        return ErrorCode::S_OK;
    }

    /**
     * @brief 读取属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @return T 属性值
     */
    operator T() const
    {
        return safe_get();
    }

    /**
     * @brief 获取属性的引用
     *
     * @note 此方法非线程安全
     *
     * @return T& 属性值引用
     */
    T& ref()
    {
        return _value;
    }

    /**
     * @brief 获取属性值的地址
     *
     * @note 此方法非线程安全
     *
     * @return T* 属性值地址
     */
    T* operator&()
    {
        return (T*)&_value;
    }

    /**
     * @brief 写入属性值
     *
     * @note 此方法线程安全
     * @note 不能在中断函数内使用
     *
     * @tparam K
     * @param other
     * @return auto&
     */
    template <PropertyVal K>
    auto& operator=(const K other)
    {
        safe_set(other);
        return *this;
    }

    virtual ErrorCode get(Extra& extra) override
    {
        extra.reset();
        if (!extra.add(safe_get())) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(Extra& extra) override
    {
        ErrorCode err;
        T         value;
        if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;
        if ((err = safe_set(value)) != ErrorCode::S_OK) return err;
        extra.reset();
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra) override
    {
        extra.reset();
        extra.add<uint16_t>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T _value;
};
