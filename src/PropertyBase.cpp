#include "PropertyBase.hpp"
#include <mutex>

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

ErrorCode           PropertyBase::set(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}

ErrorCode PropertyBase::get_size(Extra& extra)
{
    return ErrorCode::E_NO_IMPLEMENT;
}
