#include "PropertyBase.hpp"

#if NO_LOCK

struct MutexDefault : public Mutex
{
    virtual void lock() override
    {
    }

    virtual void unlock() override
    {
    }
};

static MutexDefault LockDefault;
Mutex&              PropertyBase::MutexGlobal = LockDefault;

#endif

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
