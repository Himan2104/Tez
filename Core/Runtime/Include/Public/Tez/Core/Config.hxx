#pragma once

#ifndef TEZ_VERSION_MAJOR
    #define TEZ_VERSION_MAJOR 0
    #define TEZ_VERSION_MINOR 1
    #define TEZ_VERSION_PATCH 4
#endif

#if defined(_WIN32)
    #define TEZ_PLATFORM_WINDOWS
#elif defined(__unix__)
    #if defined(__linux__)
        #define TEZ_PLATFORM_LINUX
    #else
        #error "Unsupported OS"
    #endif
#else
    #error "Unsupported OS"
#endif

#if defined(TEZ_PLATFORM_WINDOWS)
    #define TEZ_EXPORT __declspec(dllexport)
    #define TEZ_IMPORT __declspec(dllimport)
#elif defined(TEZ_PLATFORM_LINUX)
    #if __GNUC__ >= 4
        #define TEZ_EXPORT __attribute__((__visibility__("default")))
        #define TEZ_IMPORT __attribute__((__visibility__("default")))
    #else
        #define TEZ_EXPORT
        #define TEZ_IMPORT
    #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define TEZ_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define TEZ_FUNC_SIG __FUNCSIG__
#else
    #error "Unknown Compiler! Could not resolve TEZ_FUNC_SIG"
#endif

// TODO: remove this hard define
#define TEZ_ENABLE_IMGUI
