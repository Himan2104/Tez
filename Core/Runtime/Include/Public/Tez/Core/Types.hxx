#pragma once

#include "Config.hxx"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdfloat>
#include <string_view>

namespace Tez
{

using Bool = bool;

using Char = char;

using Byte = std::byte;

using Int8  = std::int8_t;
using Int16 = std::int16_t;
using Int32 = std::int32_t;
using Int64 = std::int64_t;

using UInt8  = std::uint8_t;
using UInt16 = std::uint16_t;
using UInt32 = std::uint32_t;
using UInt64 = std::uint64_t;

template <std::integral T>
struct Limits
{
    static constexpr T min = std::numeric_limits<T>::min;
    static constexpr T max = std::numeric_limits<T>::max();
};

#ifdef __STDCPP_FLOAT16_T__
using Float16 = std::float16_t;
#endif

#ifdef __STDCPP_FLOAT32_T__
using Float32 = std::float32_t;
#else
using Float32 = float;
#endif

#ifdef __STDCPP_FLOAT64_T__
using Float64 = std::float64_t;
#else
using Float64 = double;
#endif

#ifdef __STDCPP_FLOAT128_T__
using Float128 = std::float128_t;
#endif

template <typename T>
constexpr std::string_view NameOf()
{
#if defined(__clang__) || defined(__GNUC__)
    std::string_view name   = TEZ_FUNC_SIG;
    std::string_view prefix = "[with T = ";
    std::string_view suffix = ";";
#elif defined(_MSC_VER)
    std::string_view name   = TEZ_FUNC_SIG;
    std::string_view prefix = "NameOf<";
    std::string_view suffix = ">(void)";
#endif

    size_t start = name.find(prefix) + prefix.size();
    size_t end   = name.find(suffix, start);
    return name.substr(start, end - start);
}

template <typename T>
constexpr UInt64 TypeID()
{
    // FNV1A based hashing
    auto name          = NameOf<T>();
    UInt64 hash        = 0xcbf29ce484222325;
    const UInt64 prime = 0x100000001b3;

    for (char c : name)
    {
        hash ^= static_cast<UInt64>(c);
        hash *= prime;
    }
    return hash;
}

} // namespace Tez
