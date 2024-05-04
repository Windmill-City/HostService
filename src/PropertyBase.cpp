#include "PropertyBase.hpp"

AES PropertyBase::AES;

#ifndef NO_LOCK
std::recursive_mutex PropertyBase::Mutex;
#endif

ErrorCode PropertyBase::set(Extra& extra)
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

ErrorCode PropertyBase::get_desc(Extra& extra)
{
    auto name = this->name;
    auto size = strlen(name);
    extra.add(name, size);
    return ErrorCode::S_OK;
}