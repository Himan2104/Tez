#pragma once

#include "Log.hxx"
#include <cassert>
#include <cstdlib>
#include <type_traits>

#ifndef TEZ_DISABLE_ASSERTS

    #define TEZ_STATIC_ASSERT(cond, msg) static_assert(cond, msg);

    #define TEZ_SOFT_ASSERT(cond, msg, ...)                                   \
        do                                                                    \
        {                                                                     \
            if (!(cond))                                                      \
            {                                                                 \
                Tez::LogSystem::GetInstance().Log(msg, Tez::LogType::ASSERT); \
                return __VA_ARGS__;                                           \
            }                                                                 \
        } while (0)

    #define TEZ_HARD_ASSERT(cond, msg)                                        \
        do                                                                    \
        {                                                                     \
            if (!(cond))                                                      \
            {                                                                 \
                Tez::LogSystem::GetInstance().Log(msg, Tez::LogType::ASSERT); \
                std::abort();                                                 \
            }                                                                 \
        } while (0)

#else

    #define TEZ_STATIC_ASSERT(...)
    #define TEZ_SOFT_ASSERT(...)
    #define TEZ_HARD_ASSERT(...)

#endif
