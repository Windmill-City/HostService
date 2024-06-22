#pragma once
#include "Property.hpp"

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
template <Number T, Access _access = Access::READ_WRITE>
struct Range : public Property<RangeVal<T>, _access>
{
    using parent = Property<RangeVal<T>, _access>;

    const RangeVal<T> Absolute;

    Range(RangeVal<T>& value, RangeVal<T> absolute)
        : parent(value)
        , Absolute(absolute)
    {
    }

    virtual ErrorCode set(Extra& extra, bool) override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;

        switch (access)
        {
        case RangeAccess::Range:
        {
            RangeVal<T> value;
            if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;
            if (value.min > value.max) return ErrorCode::E_INVALID_ARG;
            if (value.min < Absolute.min) return ErrorCode::E_OVER_LOW_LIMIT;
            if (value.max > Absolute.max) return ErrorCode::E_OVER_HIGH_LIMIT;

            this->_value = value;
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

    virtual ErrorCode get(Extra& extra, bool) const override
    {
        RangeAccess access;
        if (!extra.get(access)) return ErrorCode::E_INVALID_ARG;
        extra.reset();

        switch (access)
        {
        case RangeAccess::Range:
            extra.add(this->_value);
            return ErrorCode::S_OK;
        case RangeAccess::Absolute:
            extra.add(Absolute);
            return ErrorCode::S_OK;
        default:
            return ErrorCode::E_NO_IMPLEMENT;
        }
        return ErrorCode::E_INVALID_ARG;
    }

    virtual ErrorCode get_size(Extra& extra, bool) const override
    {
        extra.reset();
        extra.add<Size>(sizeof(this->_value));
        return ErrorCode::S_OK;
    }
};
