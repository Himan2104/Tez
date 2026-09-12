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

///
/// @brief Logs an informational message through the LogSystem.
///
#define TEZ_LOG_INFO(message) Tez::LogSystem::GetInstance().Log(message, LogType::INFO)

///
/// @brief Logs a warning message through the LogSystem.
///
#define TEZ_LOG_WARNING(message) Tez::LogSystem::GetInstance().Log(message, LogType::WARNING)

///
/// @brief Logs an error message through the LogSystem.
/// @param message A string literal or std::string_view message body.
///
#define TEZ_LOG_ERROR(message) Tez::LogSystem::GetInstance().Log(message, LogType::ERROR)

namespace Tez
{

///
/// @brief A single log record carrying severity, message body, and a capture timestamp.
///
struct LogEntry
{
    ///
    /// @brief Severity level of this entry.
    ///
    LogType logType{LogType::INFO};

    ///
    /// @brief The textual content of the log entry.
    ///
    std::string_view message{};

    ///
    /// @brief Timestamp at which the entry was produced.
    ///
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};

///
/// @brief Abstract sink interface for log output. Derived classes are registered with the
///        LogSystem and receive every entry as it is written.
///
class ILogChannel
{
public:
    ///
    /// @brief Default constructor.
    ///
    ILogChannel() = default;

    ///
    /// @brief Virtual destructor ensuring derived channels are released correctly.
    ///
    virtual ~ILogChannel() = default;

    ///
    /// @brief Receives a log entry as soon as it is produced by the LogSystem.
    /// @param log The entry to deliver.
    ///
    virtual void OnLogReceived(const LogEntry& log) = 0;
};

///
/// @brief Concept constraining a type to be a concrete log channel.
/// @tparam T The type checked for channel compatibility.
///
template <typename T>
concept IsChannel = std::is_base_of_v<ILogChannel, T>;

///
/// @brief Central, process-wide log dispatcher. Fans out every LogEntry to all registered
///        channels and retains a bounded in-memory buffer of recent entries.
///
class LogSystem
{
public:
    ///
    /// @brief Retrieves the process-wide singleton instance of the LogSystem.
    /// @return LogSystem& A reference to the sole LogSystem instance.
    ///
    static LogSystem& GetInstance();

    ///
    /// @brief Registers a concrete channel type for future log delivery.
    /// @note Re-registering an already-existing channel is deduplicated and skipped with a
    ///       warning log.
    /// @tparam T A type satisfying IsChannel.
    /// @param args Constructor arguments forwarded to the channel's constructor.
    ///
    template <IsChannel T, typename... Args>
    void AddChannel(Args&&... args);

    ///
    /// @brief Emits a log entry at the given severity to all registered channels.
    /// @param msg The message body to log.
    /// @param logType The severity under which the entry is delivered.
    ///
    void Log(std::string_view msg, LogType logType);

private:
    ///
    /// @brief Private constructor; obtain the instance via GetInstance().
    ///
    LogSystem() = default;

    ///
    /// @brief Maximum number of entries retained in the in-memory ring buffer.
    ///
    UInt32 _bufferSize{0xFFFFFFFFLL};

    ///
    /// @brief Retained recent entries, bounded by _bufferSize.
    ///
    std::vector<LogEntry> _logs{};

    ///
    /// @brief Registered channels, keyed by type id to keep registration unique per T.
    ///
    std::vector<std::pair<UInt64, std::unique_ptr<ILogChannel>>> _logChannels{};
};

///
/// @brief Registers a concrete channel type for future log delivery (see LogSystem::AddChannel).
/// @tparam T A type satisfying IsChannel.
/// @tparam Args Deduced constructor argument types.
/// @param args Constructor arguments forwarded to the channel's constructor.
///
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

///
/// @brief std::formatter specialisation rendering a LogEntry as a bracketed timestamp,
///        severity, and message body.
///
template <>
struct std::formatter<Tez::LogEntry> : std::formatter<std::string>
{
    ///
    /// @brief Formats the entry as "[YYYY-mm-dd HH:MM:SS] [SEVERITY] message".
    /// @param log The entry to serialise.
    /// @param ctx The format context to write into.
    /// @return format_parse_context::iterator Iterator past the written characters.
    ///
    auto format(const Tez::LogEntry& log, std::format_context& ctx) const
    {
        return std::formatter<std::string>::format(
            std::format("[{:%Y-%m-%d %X}] [{}] {}", log.timestamp, log.logType, log.message), ctx);
    }
};
