#pragma once
#include <CProperty.hpp>
#include <HostClient.hpp>
#include <Range.hpp>

/**
 * @brief 范围属性值模板(客户端)
 *
 * 设置命令:
 * Range,Min,Max
 * 读取命令:
 * Range,Min,Max
 * Absolute,Min,Max
 *
 * @tparam T 数值类型
 * @tparam access 访问级别
 */
template <Number T, Access _access = Access::READ_WRITE>
struct CRange
{
    // 绑定的属性值名称
    const frozen::string name;

    CRange(const frozen::string name)
        : name(name)
    {
    }

    ErrorCode get(HostClient& client, RangeAccess access, RangeVal<T>& value) const
    {
        ErrorCode err;
        Extra&    extra   = client.extra;
        // 是否需要加密
        bool      encrypt = _access == Access::READ_WRITE_PROTECT || _access == Access::READ_PROTECT;

        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问类型
        extra.add(access);
        // 发送请求
        client.send(Command::GET_PROPERTY, extra, encrypt);
        if (err != ErrorCode::S_OK) return err;
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 读取属性值
        if (!extra.get(value)) return ErrorCode::E_FAIL;
        return ErrorCode::S_OK;
    }

    ErrorCode set(HostClient& client, const RangeVal<T>& value) const
    {
        if (_access == Access::READ || _access == Access::READ_PROTECT) return ErrorCode::E_READ_ONLY;

        ErrorCode err;
        Extra&    extra   = client.extra;
        // 是否需要加密
        bool      encrypt = _access == Access::READ_WRITE_PROTECT || _access == Access::WRITE_PROTECT;

        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问类型
        extra.add(RangeAccess::Range);
        // 添加数据
        extra.add(value);
        // 发送请求
        client.send(Command::SET_PROPERTY, extra, encrypt);
        if (err != ErrorCode::S_OK) return err;
        // 接收响应
        if (!client.recv_response(Command::SET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        return ErrorCode::S_OK;
    }
};
