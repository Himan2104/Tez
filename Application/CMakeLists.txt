include(${CMAKE_SOURCE_DIR}/Tez.cmake)

tez_lib_target(Application
    SOURCES
        Runtime/Source/Window.cxx

    PUBLIC_INCLUDES Runtime/Include/Public

    PRIVATE_INCLUDES Runtime/Include/Private

    PUBLIC_DEPENDENCIES
        Tez::Core

    PRIVATE_DEPENDENCIES
        glfw
    )
