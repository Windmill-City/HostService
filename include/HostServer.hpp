#pragma once
#include "Types.hpp"
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <frozen/string.h>
#include <HostBase.hpp>

template <size_t _size>
using PropertyMap = std::array<std::pair<frozen::string, PropertyBase*>, _size>;

struct PropertyHolderBase
{
    /**
     * @brief 根据属性id获取属性值
     *
     * @param id 属性id
     * @return PropertyBase* 指向属性值的指针
     */
    virtual PropertyBase*  get(PropertyId id) const      = 0;
    /**
     * @brief 根据属性id获取属性值描述
     *
     * @param id 属性id
     * @return 属性值描述
     */
    virtual frozen::string get_desc(PropertyId id) const = 0;
    /**
     * @brief 获取属性值个数
     *
     * @return size_t 属性值个数
     */
    virtual size_t         size() const                  = 0;
};

struct PropertySymbols : public PropertyAccess<Access::READ>
{
    virtual ErrorCode get(Extra& extra, bool privileged) const override
    {
        PropertyId id;
        if (!extra.get(id)) return ErrorCode::E_INVALID_ARG;
        if (id >= _holder->size()) return ErrorCode::E_OUT_OF_INDEX;

        PropertyBase* prop = _holder->get(id);
        ErrorCode     err  = prop->check_read(privileged);
        if (err != ErrorCode::S_OK) return err;

        extra.reset();
        frozen::string desc = _holder->get_desc(id);
        extra.add(desc.data(), desc.size());

        return ErrorCode::S_OK;
    }

    virtual ErrorCode get_size(Extra& extra, bool) const override
    {
        if (_holder->size() > UINT16_MAX) return ErrorCode::E_OUT_OF_INDEX;
        extra.reset();
        extra.add<Size>(_holder->size());
        return ErrorCode::S_OK;
    }

    PropertyHolderBase* _holder = nullptr;
};

template <size_t _size>
struct PropertyHolder : public PropertyHolderBase
{
    using Map = PropertyMap<_size>;
    const Map& map;

    PropertyHolder(const Map& map)
        : map(map)
    {
    }

    PropertyHolder(const Map& map, PropertySymbols& ids)
        : map(map)
    {
        ids._holder = this;
    }

    virtual PropertyBase* get(PropertyId id) const override
    {
        if (id >= map.size()) return nullptr;
        return map[id].second;
    }

    virtual frozen::string get_desc(PropertyId id) const override
    {
        if (id >= map.size()) return "";
        return map[id].first;
    }

    virtual size_t size() const override
    {
        return map.size();
    }
};

struct HostServer : public HostBase
{
    HostServer(Address& address, const PropertyHolderBase& holder, SecretHolder& secret)
        : HostBase(address, secret)
        , _holder(holder)
    {
    }

    bool poll();
    void log(const void* log, size_t size);

  protected:
    PropertyBase* _acquire_and_verify(Command cmd, Extra& extra, bool encrypted);

  protected:
    // 附加参数缓冲区
    Extra                     _extra;
    // 属性值容器
    const PropertyHolderBase& _holder;
};
