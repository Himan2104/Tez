#pragma once

#include "Tez/Core/Math.hxx"
#include <Tez/Application/VideoMode.hxx>
#include <Tez/Core/Array.hxx>
#include <Tez/Core/Math.hxx>
#include <Tez/Core/Span.hxx>
#include <Tez/Core/Types.hxx>
#include <format>
#include <string>
#include <string_view>

namespace Tez
{

class Monitor
{
public:
    Monitor() = delete;
    Monitor(std::shared_ptr<class MonitorImpl>);

    [[nodiscard]] Span<const VideoMode> GetSupportedVideoModes() const;
    [[nodiscard]] const VideoMode& GetCurrentVideoMode() const;
    [[nodiscard]] std::string_view GetName() const;
    [[nodiscard]] Vec2i GetPosition() const;
    [[nodiscard]] Vec4i GetWorkArea() const;
    [[nodiscard]] Vec2i GetPhysicalSize() const;
    [[nodiscard]] Vec4f GetContentScale() const;

    [[nodiscard]] static const DynamicArray<Monitor> GetMonitors();
    [[nodiscard]] static Monitor GetPrimaryMonitor();

private:
    std::shared_ptr<class MonitorImpl> _monitorImpl;
};

} // namespace Tez

template <>
struct std::formatter<Tez::Monitor> : std::formatter<std::string>
{

    constexpr auto format(const Tez::Monitor& monitor, std::format_context& ctx)
    { return std::format_to(ctx.out(), "{}", monitor.GetName()); }
};
