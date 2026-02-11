#include <Tez/Core/Log.hxx>
#include <Tez/Core/LogType.hxx>
#include <chrono>
#include <string_view>

namespace Tez
{
LogSystem& LogSystem::GetInstance()
{
    static LogSystem instance;
    return instance;
}

void LogSystem::Log(std::string_view msg, LogType type)
{
    LogEntry log{.logType = type, .message = msg, .timestamp = std::chrono::system_clock::now()};
    for (auto& channel : _logChannels) { channel.second->OnLogReceived(log); }
}
} // namespace Tez
