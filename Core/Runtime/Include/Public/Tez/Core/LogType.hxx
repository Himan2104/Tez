#pragma once

#include <format>

namespace Tez
{
enum class LogType
{
    UNKNOWN = 0,
    INFO    = 1,
    WARNING = 2,
    ERROR   = 3,
    ASSERT  = 4
};
} // namespace Tez

template <>
struct std::formatter<Tez::LogType> : std::formatter<std::string_view>
{
    auto format(Tez::LogType logType, std::format_context& ctx) const
    {
        std::string_view name;
        switch (logType)
        {
        case Tez::LogType::UNKNOWN: name = "UNKNOWN"; break;
        case Tez::LogType::INFO: name = "INFO"; break;
        case Tez::LogType::WARNING: name = "WARNING"; break;
        case Tez::LogType::ERROR: name = "ERROR"; break;
        case Tez::LogType::ASSERT: name = "ASSERT"; break;
        }
        return std::formatter<std::string_view>::format(name, ctx);
    }
};
