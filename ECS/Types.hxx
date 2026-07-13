#pragma once

#include <Tez/Core/Types.hxx>

namespace Tez
{
using Entity = UInt64;

constexpr Entity InvalidEntity = 0;

struct IComponent
{
};

struct ITag
{
};

class ISystem
{
public:
    virtual ~ISystem() = default;

protected:
    virtual void Init()                  = 0;
    virtual void Tick(Float32 deltaTime) = 0;
    virtual void Kill()                  = 0;

private:
    friend class EntityManager;
};

template <typename T>
class System : public ISystem
{
};
} // namespace Tez
