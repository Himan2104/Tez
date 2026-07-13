#pragma once

#include <Tez/Core/Array.hxx>
#include <type_traits>

namespace Tez
{

constexpr int kInvalidUID = 0;

template <typename T>
    requires std::is_integral_v<T>
class UIDProvider final
{
public:
    constexpr UIDProvider() = default;

    [[nodiscard]] constexpr T GetNewUID()
    { return _freeIDs.IsEmpty() ? ++_counter : _freeIDs.PopBack().value(); }

    constexpr void FreeID(T id) { _freeIDs.PushBack(id); }

private:
    DynamicArray<T> _freeIDs;
    T _counter{};
};

} // namespace Tez
