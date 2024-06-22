#pragma once
#include "Types.hpp"
#include <cstddef>
#include <cstdint>
#include <HostClient.hpp>
#include <Memory.hpp>

/**
 * @brief 内存区属性(客户端)
 *
 * @note 内存区属性的读写分块进行, 需要额外的机制来保障数据的完整性
 *
 * @tparam T 属性类型
 * @tparam access 访问级别
 */
template <PropertyVal T, Access _access = Access::READ_WRITE>
struct CMemory
{
    // 绑定的属性名称
    const frozen::string name;

    CMemory(const frozen::string name)
        : name(name)
    {
    }

    ErrorCode set_block(HostClient& client, MemoryAccess& access, const uint8_t* data, bool encrypt) const
    {
        ErrorCode err;
        Extra&    extra = client.extra;
        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问参数
        extra.add(access);
        // 添加数据
        extra.add(data, access.size);
        // 发送请求
        client.send(Command::SET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::SET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 自增偏移
        access.offset += access.size;
        return ErrorCode::S_OK;
    }

    ErrorCode get_block(HostClient& client, MemoryAccess& access, uint8_t* data, bool encrypt)
    {
        ErrorCode err;
        Extra&    extra = client.extra;
        extra.reset();
        // 添加id
        PropertyId id;
        err = client.holder.get_id_by_name(name, id);
        if (err != ErrorCode::S_OK) return err;
        extra.add(id);
        // 添加访问参数
        extra.add(access);
        // 发送请求
        client.send(Command::GET_PROPERTY, extra, encrypt);
        // 接收响应
        if (!client.recv_response(Command::GET_PROPERTY, err, extra)) return ErrorCode::E_TIMEOUT;
        if (err != ErrorCode::S_OK) return err;
        // 接收数据
        if (!extra.get(data, access.size)) return ErrorCode::E_FAIL;
        // 自增偏移
        access.offset += access.size;
        return ErrorCode::S_OK;
    }

    ErrorCode set(HostClient& client, Size offset, const void* buffer, size_t size) const
    {
        if (_access == Access::READ || _access == Access::READ_PROTECT) return ErrorCode::E_READ_ONLY;

        // 是否需要加密
        bool         encrypt = _access == Access::READ_WRITE_PROTECT || _access == Access::WRITE_PROTECT;
        // 每次同步的最大长度
        Size         space   = MEMORY_ACCESS_SIZE_MAX;
        // buffer的偏移
        Size         _offset = 0;
        // 内存访问参数
        MemoryAccess access;
        access.size   = space;
        access.offset = offset;
        // 本地数据访问指针
        uint8_t* data = (uint8_t*)buffer;

        // 分块发送数据 - 整数倍部分
        for (size_t i = 0; i < size / space; i++)
        {
            ErrorCode err = set_block(client, access, &data[_offset], encrypt);
            if (err != ErrorCode::S_OK) return err;
            _offset += access.size;
        }

        // 分块发送数据 - 余下的部分
        if (_offset != size)
        {
            access.size   = size - _offset;
            ErrorCode err = set_block(client, access, &data[_offset], encrypt);
            if (err != ErrorCode::S_OK) return err;
        }

        return ErrorCode::S_OK;
    }

    ErrorCode get(HostClient& client, Size offset, void* buffer, size_t size)
    {
        // 是否需要加密
        bool         encrypt = _access == Access::READ_WRITE_PROTECT || _access == Access::READ_PROTECT;
        // 每次同步的最大长度
        Size         space   = MEMORY_ACCESS_SIZE_MAX;
        // buffer 的偏移
        Size         _offset = 0;
        // 内存访问参数
        MemoryAccess access;
        access.size   = space;
        access.offset = offset;
        // 本地数据访问指针
        uint8_t* data = (uint8_t*)buffer;

        // 分块读取数据 - 整数倍部分
        for (size_t i = 0; i < size / space; i++)
        {
            ErrorCode err = get_block(client, access, &data[_offset], encrypt);
            if (err != ErrorCode::S_OK) return err;
            _offset += access.size;
        }

        // 分块读取数据 - 余下的部分
        if (_offset != size)
        {
            access.size   = size - _offset;
            ErrorCode err = get_block(client, access, &data[_offset], encrypt);
            if (err != ErrorCode::S_OK) return err;
        }
        return ErrorCode::S_OK;
    }
};