#include "HostClient.hpp"

/**
 * @brief 接收响应
 *
 * @param cmd 期望的命令
 * @param err 错误码
 * @param extra 附加参数
 * @return true 成功接收一帧
 * @return false 接收超时
 */
bool HostClient::recv_response(const Command cmd, ErrorCode& err, Extra& extra)
{
Start:
    Command r_cmd;
    if (!recv(r_cmd, err, extra)) return false;

    // 验证命令
    if (r_cmd != cmd)
    {
        // 输出 log 信息
        if (r_cmd == Command::LOG)
        {
            log_output((const char*)extra.data(), extra.remain());
        }
        goto Start;
    }
    return true;
}