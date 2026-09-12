#pragma once

#include <Tez/Core/Array.hxx>
#include <type_traits>

namespace Tez
{

constexpr int kInvalidUID = 0;

///
/// @brief Issues and recycles uniquely-identifying values for a given integral type.
/// @note A freed UID is re-issued before the monotonic counter advances, so recycled
///       ids stay unique while an id is outstanding.
/// @tparam T Any integral type large enough to hold the desired id range.
///
template <typename T>
    requires std::is_integral_v<T>
class UIDProvider final
{
public:
    constexpr UIDProvider() = default;

    ///
    /// @brief Fetches the next available UID, reusing a freed one if possible.
    /// @return T The newly issued unique identifier.
    ///
    [[nodiscard]] constexpr T GetNewUID()
    { return _freeIDs.IsEmpty() ? ++_counter : _freeIDs.PopBack().value(); }

    ///
    /// @brief Returns a previously issued UID to the pool for later re-issue.
    /// @param id The identifier to recycle.
    ///
    constexpr void FreeID(T id) { _freeIDs.PushBack(id); }

private:
    DynamicArray<T> _freeIDs;
    T _counter{};
};

} // namespace Tez
