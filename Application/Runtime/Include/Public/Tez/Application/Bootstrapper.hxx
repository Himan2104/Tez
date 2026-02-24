#pragma once

#include <Tez/Application/IApplication.hxx>
#include <utility>

namespace Tez
{
class Bootstrapper final
{
public:
    template <typename T, typename... Args>
    static int Start(Args&&... args)
    {
        _application = new T(std::forward(args...));
    }

private:
    static IApplication* _application;
};
} // namespace Tez
