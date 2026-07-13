#pragma once

#include <Tez/Core/Array.hxx>
#include <Tez/Core/Math.hxx>
#include <Tez/Core/Types.hxx>
#include <format>
#include <string>

namespace Tez
{
struct VideoMode
{
    Int32 width;
    Int32 height;
    Int32 redBits;
    Int32 greenBits;
    Int32 blueBits;
    Int32 refreshRate;
};
} // namespace Tez

template <>
struct std::formatter<Tez::VideoMode> : std::formatter<std::string>
{
    auto format(const Tez::VideoMode& vm, std::format_context& ctx)
    {
        return std::format_to(ctx.out(), "{}x{}@{}[R{}G{}B{}]", vm.width, vm.height, vm.refreshRate,
                              vm.redBits, vm.greenBits, vm.blueBits);
    }
};
