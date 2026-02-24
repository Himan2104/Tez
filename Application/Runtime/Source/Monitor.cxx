#include "GLFW/glfw3.h"
#include "Tez/Application/VideoMode.hxx"
#include "Tez/Core/Array.hxx"
#include "Tez/Core/Span.hxx"
#include "Tez/Core/Vector2.hxx"
#include "Tez/Core/Vector4.hxx"
#include <Tez/Application/Monitor.hxx>
#include <memory>
#include <string_view>

namespace Tez
{

class MonitorImpl
{
public:
    MonitorImpl(GLFWmonitor* monitor)
        : _monitor{monitor}
    {
    }

    Vector2i GetPosition()
    {
        Vector2i pos;
        glfwGetMonitorPos(_monitor, &pos.x, &pos.y);
        return pos;
    }

    Vector4i GetWorkArea()
    {
        Vector4i workArea;
        glfwGetMonitorWorkarea(_monitor, &workArea.x, &workArea.y, &workArea.z, &workArea.w);
        return workArea;
    }

    Vector2i GetPhysicalSize()
    {
        Vector2i size;
        glfwGetMonitorPhysicalSize(_monitor, &size.x, &size.y);
        return size;
    }

    Vector2f GetContentScale()
    {
        Vector2f scale;
        glfwGetMonitorContentScale(_monitor, &scale.x, &scale.y);
        return scale;
    }

    std::string_view GetName() { return std::string_view(glfwGetMonitorName(_monitor)); }

    const VideoMode* GetCurrentVideoMode() const
    {
        const GLFWvidmode* gvm = glfwGetVideoMode(_monitor);
        return reinterpret_cast<const VideoMode*>(gvm);
    }

    Span<const VideoMode> GetSupportedVideoModes() const
    {
        int count;
        const GLFWvidmode* vidModes = glfwGetVideoModes(_monitor, &count);

        return Span<const VideoMode>(reinterpret_cast<const VideoMode*>(vidModes), count);
    }

private:
    GLFWmonitor* _monitor{nullptr};
};

Monitor::Monitor(std::shared_ptr<MonitorImpl> monitorImpl)
    : _monitorImpl{monitorImpl}
{
}

Span<const VideoMode> Monitor::GetSupportedVideoModes() const
{
    return _monitorImpl->GetSupportedVideoModes();
}

const DynamicArray<Monitor> Monitor::GetMonitors()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    DynamicArray<Monitor> ret;
    ret.Reserve(count);

    for (int i = 0; i < count; i++) { ret.EmplaceBack(std::make_shared<MonitorImpl>(monitors[i])); }
    return ret;
}

Monitor Monitor::GetPrimaryMonitor()
{
    return Monitor(std::make_shared<MonitorImpl>(glfwGetPrimaryMonitor()));
}

} // namespace Tez
