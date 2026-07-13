#include <GLFW/glfw3.h>
#include <Tez/Application/Window.hxx>

namespace Tez
{
class WindowImpl
{
public:
    WindowImpl(const WindowLaunchProperties& props)
    {
        // TODO: Load the props and monitor

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        _window = glfwCreateWindow(props.videoMode.width, props.videoMode.height,
                                   props.title.c_str(), glfwGetPrimaryMonitor(), NULL);
    }

    void SetVideoMode(VideoMode vm) { /*glfwWindowHint(GLFW_, int value);*/ }

private:
    GLFWwindow* _window{nullptr};
};

} // namespace Tez
