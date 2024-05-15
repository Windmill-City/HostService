#pragma once
#include <Extra.hpp>
#include <FixedQueue.hpp>
#include <frozen/map.h>
#include <frozen/string.h>
#include <HostBase.hpp>

template <size_t _size>
using CPropertyMap = frozen::map<frozen::string, PropertyId, _size>;

struct HostClient;

struct CPropertyHolderBase
{
    /**
     * @brief 根据属性名获取属性Id
     *
     * @details
     * 客户端连接服务端后, 首先通过客户端的 0号变量 symbols 获取 id表;
     * 获取 id表后, 客户端将id表缓存下来, 每次请求时都从此表查id
     *
     * @param name 属性名
     * @param id 属性Id
     * @return ErrorCode
     */
    virtual ErrorCode get_id_by_name(const frozen::string name, PropertyId& id) = 0;
    /**
     * @brief 刷新id表
     *
     * @param client 客户端实例
     * @return ErrorCode
     */
    virtual ErrorCode refresh(HostClient& client)                               = 0;
};

template <size_t _size>
struct CPropertyHolder : public CPropertyHolderBase
{
    using Map = CPropertyMap<_size>;
    Map& map;

    CPropertyHolder(Map& map)
        : map(map)
    {
    }

    virtual ErrorCode get_id_by_name(const frozen::string name, PropertyId& id) override
    {
        if (map.contains(name))
        {
            id = map.at(name);
            return ErrorCode::S_OK;
        }
        return ErrorCode::E_ID_NOT_EXIST;
    }

    virtual ErrorCode refresh(HostClient& client) override
    {
        return ErrorCode::S_OK;
    }
};

struct HostClient : public HostBase
{
    // 帧头缓冲区
    Sync<Response>       _buf;
    // 附加参数缓冲区
    Extra                extra;
    // 随机数
    NonceType            nonce;
    // 通信密钥
    KeyType              key;
    // 属性值Id容器
    CPropertyHolderBase& holder;

    HostClient(const PropertyAddress& addr, CPropertyHolderBase& holder)
        : HostBase(addr)
        , holder(holder)
    {
    }

    void send_request(const Command cmd, Extra& extra, bool encrypt = false);
    bool recv_response(Command cmd, ErrorCode& err, Extra& extra);
};
