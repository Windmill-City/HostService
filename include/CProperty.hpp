#pragma once
#include <HostClient.hpp>
#include <Property.hpp>

/**
 * @brief 属性值模板(客户端)
 *
 *
 * @tparam T 数值类型
 * @tparam access 访问级别
 */
template <PropertyVal T, Access access = Access::READ_WRITE>
struct CProperty
{
    // 绑定的属性名称
    const frozen::string name;

    CProperty(const frozen::string name)
        : name(name)
    {
    }

    ErrorCode get(HostClient& client, T& value) const
    {
        ErrorCode err;
        Extra&    extra   = client.extra;
        // 是否需要加密
        bool      encrypt = access == Access::READ_WRITE_PROTECT || access == Access::READ_PROTECT;

        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 发送请求
        client.send(Command::GET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 读取数据
        if (!extra.get(value)) return ErrorCode::E_FAIL;
        return ErrorCode::S_OK;
    }

    ErrorCode set(HostClient& client, const T value) const
    {
        if (access == Access::READ || access == Access::READ_PROTECT) return ErrorCode::E_READ_ONLY;

        ErrorCode err;
        Extra&    extra   = client.extra;
        // 是否需要加密
        bool      encrypt = access == Access::READ_WRITE_PROTECT || access == Access::WRITE_PROTECT;

        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加数据
        if (!extra.add(value)) return ErrorCode::E_OUT_OF_BUFFER;
        // 发送请求
        client.send(Command::SET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::SET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        return ErrorCode::S_OK;
    }
};
