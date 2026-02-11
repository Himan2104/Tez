#pragma once

#include "LogType.hxx"
#include "Types.hxx"
#include <algorithm>
#include <chrono>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#define TEZ_LOG_INFO(message)    Tez::LogSystem::GetInstance().Log(message, LogType::INFO)
#define TEZ_LOG_WARNING(message) Tez::LogSystem::GetInstance().Log(message, LogType::WARNING)
#define TEZ_LOG_ERROR(message)   Tez::LogSystem::GetInstance().Log(message, LogType::ERROR)

namespace Tez
{

struct LogEntry
{
    LogType logType{LogType::INFO};
    std::string_view message{};
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};

class ILogChannel
{
public:
    ILogChannel()                                   = default;
    virtual ~ILogChannel()                          = default;
    virtual void OnLogReceived(const LogEntry& log) = 0;
};

template <typename T>
concept IsChannel = std::is_base_of_v<ILogChannel, T>;

class LogSystem
{
public:
    static LogSystem& GetInstance();

    template <IsChannel T, typename... Args>
    void AddChannel(Args&&... args);

    void Log(std::string_view msg, LogType logType);

private:
    LogSystem() = default;

    UInt32 _bufferSize{0xFFFFFFFFLL};
    std::vector<LogEntry> _logs{};
    std::vector<std::pair<UInt64, std::unique_ptr<ILogChannel>>> _logChannels{};
};

template <IsChannel T, typename... Args>
void LogSystem::AddChannel(Args&&... args)
{
    if (std::find_if(_logChannels.begin(), _logChannels.end(), [](const auto& kvp)
                     { return kvp.first == TypeID<T>(); }) != _logChannels.end())
    {
        Log("Channel Already Exists!", LogType::WARNING);
        return;
    }

    _logChannels.push_back(
        std::pair(TypeID<T>(), std::make_unique<T>(std::forward<Args>(args)...)));
}
} // namespace Tez

template <>
struct std::formatter<Tez::LogEntry> : std::formatter<std::string>
{
    auto format(const Tez::LogEntry& log, std::format_context& ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("[{:%Y-%m-%d %X}] [{}] {}", log.timestamp, log.logType, log.message), ctx);
    }
};
