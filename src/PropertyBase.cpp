#include "PropertyBase.hpp"

ErrorCode PropertyBase::set(const uint8_t* p_value, const uint8_t size)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::set_mem(const uint16_t offset, const uint8_t* p_value, const uint8_t datlen)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get(uint8_t** p_value, uint8_t& size)
{
    size = 0;
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get_mem(const uint16_t offset, uint8_t** p_value, uint8_t& datlen)
{
    datlen = 0;
    return ErrorCode::E_NO_IMPLEMENT;
}
