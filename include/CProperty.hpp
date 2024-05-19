#pragma once
#include <cstddef>
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

    /**
     * @brief 读取属性值
     *
     * @return T 属性值
     */
    T get() const
    {
        return _value;
    }

    /**
     * @brief 设置属性值
     *
     * @param value 要写入的值
     * @return ErrorCode 错误码
     */
    ErrorCode set(T value)
    {
        _value = value;
        return ErrorCode::S_OK;
    }

    /**
     * @brief 读取属性值
     *
     * @return T 属性值
     */
    operator T() const
    {
        return get();
    }

    /**
     * @brief 获取属性的引用
     *
     * @return T& 属性值引用
     */
    T& ref()
    {
        return _value;
    }

    /**
     * @brief 获取属性值的地址
     *
     * @return T* 属性值地址
     */
    T* operator&()
    {
        return (T*)&_value;
    }

    /**
     * @brief 写入属性值
     *
     * @tparam K
     * @param other
     * @return auto&
     */
    template <PropertyVal K>
    auto& operator=(const K other)
    {
        set(other);
        return *this;
    }

    ErrorCode get(HostClient& client)
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
        client.send_request(Command::GET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 接收数据
        PropertyId id_r;
        if (!extra.get(id_r) || id != id_r) return ErrorCode::E_FAIL;
        T value;
        if (!extra.get(value)) return ErrorCode::E_FAIL;
        set(value);
        return ErrorCode::S_OK;
    }

    ErrorCode set(HostClient& client)
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
        if (!extra.add(_value)) return ErrorCode::E_OUT_OF_BUFFER;
        // 发送请求
        client.send_request(Command::SET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::SET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        return ErrorCode::S_OK;
    }

  protected:
    T _value;
};
