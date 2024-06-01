#pragma once
#include <HostClient.hpp>

template <size_t _size>
struct CPropertyHolder : public CPropertyHolderBase
{
    using Map = CPropertyMap<_size>;
    Map& map;

    CPropertyHolder(Map& map)
        : map(map)
    {
    }

    virtual ErrorCode get_id_by_name(const frozen::string name, PropertyId& id) const override
    {
        if (map.contains(name))
        {
            id = map.at(name);
            return ErrorCode::S_OK;
        }
        return ErrorCode::E_ID_NOT_EXIST;
    }

    ErrorCode get_size(HostClient& client, uint16_t& size) const
    {
        ErrorCode err;
        Extra&    extra = client.extra;

        extra.reset();
        // 添加id - symbols默认Id为 0
        extra.add<PropertyId>(0);
        // 发送请求
        client.send(Command::GET_SIZE, extra, false);
        // 接收响应
        if (!client.recv_response(Command::GET_SIZE, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 接收数据
        PropertyId id_r;
        if (!extra.get(id_r) || 0 != id_r) return ErrorCode::E_FAIL;
        if (!extra.get(size)) return ErrorCode::E_FAIL;
        return ErrorCode::S_OK;
    }

    ErrorCode get_name(HostClient& client, const PropertyId id) const
    {
        ErrorCode err;
        Extra&    extra = client.extra;

        extra.reset();
        // 添加id - symbols默认Id为 0
        extra.add<PropertyId>(0);
        // 添加id - 需要请求 symbol 的id
        extra.add<PropertyId>(id);
        // 发送请求
        client.send(Command::GET_PROPERTY, extra, false);
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 接收数据
        PropertyId id_r;
        if (!extra.get(id_r) || 0 != id_r) return ErrorCode::E_FAIL;
        if (!extra.get(id_r) || id != id_r) return ErrorCode::E_FAIL;
        return ErrorCode::S_OK;
    }

    virtual ErrorCode refresh(HostClient& client) override
    {
        ErrorCode err;
        uint16_t  size;
        uint16_t  found = 0;
        if ((err = get_size(client, size)) != ErrorCode::S_OK) return err;
        for (size_t i = 0; i < size; i++)
        {
            if (get_name(client, i) != ErrorCode::S_OK) continue;

            frozen::string name((const char*)client.extra.data(), (size_t)client.extra.remain());
            // 允许服务端拥有比客户端更多的属性
            if (!map.contains(name)) continue;
            map.at(name) = i;
            found++;
        }

        return found == map.size() ? ErrorCode::S_OK : ErrorCode::E_FAIL;
    }
};