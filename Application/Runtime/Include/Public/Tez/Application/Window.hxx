#include <Tez/Application/Monitor.hxx>
#include <Tez/Application/VideoMode.hxx>
#include <Tez/Core/Types.hxx>
#include <memory>

namespace Tez
{

struct WindowLaunchProperties
{
    enum class WindowLaunchFlags : UInt8
    {
        None        = 0,
        FullScreen  = 1 << 0,
        Decorated   = 1 << 1,
        Resizable   = 1 << 2,
        Transparent = 1 << 3,
        EnableVSync = 1 << 4,
        Maximized   = 1 << 5,
        Visible     = 1 << 6,
        Focused     = 1 << 7
    };
    std::string title;
    VideoMode videoMode;
    WindowLaunchFlags launchFlags;
    Monitor monitor;
};

class Window
{
public:
    Window() = delete;
    Window(const WindowLaunchProperties&);

private:
    std::unique_ptr<class WindowImpl> _windowImpl;
};
} // namespace Tez
