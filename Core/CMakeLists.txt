include(${CMAKE_SOURCE_DIR}/Tez.cmake)

tez_lib_target(Application
    SOURCES
    Runtime/Source/Log.cxx

    PUBLIC_INCLUDES Runtime/Include/Public
    )
