include(${CMAKE_SOURCE_DIR}/Tez.cmake)

tez_lib_target(Application
    SOURCES
        Runtime/Source/Window.cxx

    PUBLIC_INCLUDES Runtime/Include/Public>
    PUBLIC_DEPENDENCIES
        Tez::Core
    )
