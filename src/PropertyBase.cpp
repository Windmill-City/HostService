#include "PropertyBase.hpp"

std::mutex PropertyBase::mutex;

ErrorCode         PropertyBase::set(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::set_mem(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get_mem(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get_size(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}
