#pragma once
#include <CProperty.hpp>
#include <cstddef>
#include <HostClient.hpp>
#include <Range.hpp>

/**
 * @brief 范围属性值模板(客户端)
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
struct CRange
{
    // 绑定的属性值名称
    const frozen::string name;

    // 绝对范围
    RangeVal<T>          Absolute;

    CRange(const frozen::string name, const RangeVal<T> abs)
        : name(name)
        , Absolute(abs)
        , _value(abs)
    {
    }

    /**
     * @brief 获取最小值的引用
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
     * @return T& 最大值的引用
     */
    T& max()
    {
        return this->_value.max;
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

    /**
     * @brief 读取属性值
     *
     * @return T 属性值
     */
    operator RangeVal<T>() const
    {
        return get();
    }

    /**
     * @brief 获取属性的引用
     *
     * @return T& 属性值引用
     */
    RangeVal<T>& ref()
    {
        return _value;
    }

    /**
     * @brief 获取属性值的地址
     *
     * @return T* 属性值地址
     */
    RangeVal<T>* operator&()
    {
        return &_value;
    }

    /**
     * @brief 读取属性值
     *
     * @return T 属性值
     */
    virtual RangeVal<T> get() const
    {
        return _value;
    }

    /**
     * @brief 设置属性值
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    ErrorCode set(const RangeVal<T> value)
    {
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
        set(other);
        return *this;
    }

    /**
     * @brief 比较属性值
     *
     * @return true 范围相同
     * @return false 范围不同
     */
    bool operator==(const RangeVal<T>& right) const
    {
        return _value == right;
    }

    ErrorCode set(HostClient& client)
    {
        if (access == Access::READ || access == Access::READ_PROTECT) return ErrorCode::E_READ_ONLY;

        ErrorCode err;
        Extra&    extra   = client.extra;
        // 是否需要加密
        bool      encrypt = access == Access::READ_WRITE_PROTECT || access == Access::WRITE_PROTECT;

        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问类型
        extra.add(RangeAccess::Range);
        // 添加数据
        extra.add(_value);
        // 发送请求
        client.send_request(Command::SET_PROPERTY, extra, encrypt);
        if (err != ErrorCode::S_OK) return err;
        // 接收响应
        if (!client.recv_response(Command::SET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        return ErrorCode::S_OK;
    }

    ErrorCode get(HostClient& client, RangeAccess range, bool encrypt)
    {
        ErrorCode err;
        Extra&    extra = client.extra;
        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问类型
        extra.add(range);
        // 发送请求
        client.send_request(Command::GET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 接收数据
        PropertyId id_r;
        if (!extra.get(id_r) || id != id_r) return ErrorCode::E_FAIL;
        RangeAccess range_r;
        if (!extra.get(range_r) || range != range_r) return ErrorCode::E_FAIL;
        RangeVal<T> value;
        if (!extra.get(value)) return ErrorCode::E_FAIL;
        switch (range)
        {
        case RangeAccess::Range:
            return set(value);
        case RangeAccess::Absolute:
            Absolute = value;
            break;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::S_OK;
    }

    ErrorCode get(HostClient& client)
    {
        ErrorCode err;
        // 是否需要加密
        bool      encrypt = access == Access::READ_WRITE_PROTECT || access == Access::READ_PROTECT;

        // 获取绝对范围
        err               = get(client, RangeAccess::Absolute, encrypt);
        if (err != ErrorCode::S_OK) return err;
        // 获取当前范围
        err = get(client, RangeAccess::Range, encrypt);
        if (err != ErrorCode::S_OK) return err;
        return ErrorCode::S_OK;
    }

  protected:
    RangeVal<T> _value;
};

/**
 * @brief 带范围限制的属性值模板
 *
 * @tparam T 属性值类型
 * @tparam mode 限制模式
 * @tparam access 属性值的访问级
 */
template <Number T, RangeMode mode = RangeMode::Hard, Access access = Access::READ_WRITE>
struct CRangedProperty : public CProperty<T, access>
{
    using parent = CProperty<T, access>;

    CRangedProperty(const frozen::string name, const RangeVal<T>& range, T val = 0)
        : parent(name)
        , _range(range)
    {
        this->_value = val;
        clamp();
    }

    /**
     * @brief 获取最小值
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
     * @return T 最大值
     */
    T max()
    {
        return _range.max;
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
        set(other);
        return *this;
    }

    ErrorCode set(T value)
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

    ErrorCode set(HostClient& client)
    {
        return parent::set(client);
    }

    ErrorCode get(HostClient& client)
    {
        return parent::get(client);
    }

  protected:
    const RangeVal<T>& _range;
};
