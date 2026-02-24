#pragma once

namespace Tez
{
class IApplication
{
public:
    IApplication()          = default;
    virtual ~IApplication() = default;

    ///
    /// @brief Get the primary window associated with this application
    virtual class Window* GetWindow()        = 0;
    virtual void OnClose(bool force = false) = 0;

private:
};
} // namespace Tez
  //
