#pragma once

#include <optional>

namespace Tez
{
template <typename T>
using Optional = std::optional<T>; // TODO: Implment a constexpr one, unless it already is?
} // namespace Tez
