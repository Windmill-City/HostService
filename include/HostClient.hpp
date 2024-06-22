#pragma once
#include "Types.hpp"
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
    virtual ErrorCode get_id_by_name(const frozen::string name, PropertyId& id) const = 0;
    /**
     * @brief 刷新id表
     *
     * @param client 客户端实例
     * @return ErrorCode
     */
    virtual ErrorCode refresh(HostClient& client)                                     = 0;
};

struct HostClient : public HostBase
{
    // 附加参数缓冲区
    Extra                extra;
    // 属性值Id容器
    CPropertyHolderBase& holder;

    HostClient(Address address, CPropertyHolderBase& holder, SecretHolder& secret)
        : HostBase(address, secret)
        , holder(holder)
    {
    }

    bool recv_response(Command cmd, ErrorCode& err, Extra& extra);

  protected:
    /**
     * @brief 日志输出接口
     *
     * @param log 日志信息
     * @param size 日志字节长度
     */
    virtual void log_output(LogLevel level, const uint8_t* log, size_t size) = 0;
};
