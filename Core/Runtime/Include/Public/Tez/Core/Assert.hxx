#pragma once

#include "Log.hxx"
#include <cassert>
#include <cstdlib>

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
