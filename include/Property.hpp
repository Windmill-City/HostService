#pragma once
#include "PropertyBase.hpp"

/**
 * @brief 属性值模板
 *
 *
 * @tparam T 类型参数
 * @tparam access 访问级别
 */
template <PropertyVal T, Access _access = Access::READ>
struct Property : public PropertyAccess<_access>
{
    Property(T& value)
        : _value(value)
    {
    }

    virtual ErrorCode get(Extra& extra, bool) const override
    {
        extra.reset();
        if (!extra.add(this->_value)) return ErrorCode::E_OUT_OF_BUFFER;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode set(Extra& extra, bool) override
    {
        T value;
        if (!extra.get(value)) return ErrorCode::E_INVALID_ARG;
        this->_value = value;
        extra.reset();
        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra, bool) const override
    {
        extra.reset();
        extra.add<uint16_t>(sizeof(_value));
        return ErrorCode::S_OK;
    }

  protected:
    T& _value;
};
