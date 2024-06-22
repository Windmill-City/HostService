#include "PropertyBase.hpp"

ErrorCode PropertyBase::set(Extra&, bool)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get(Extra&, bool) const
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get_size(Extra&, bool) const
{
    return ErrorCode::E_NO_IMPLEMENT;
}
