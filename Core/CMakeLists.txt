include(${CMAKE_SOURCE_DIR}/Tez.cmake)

tez_lib_target(Core
    SOURCES
        Runtime/Source/Log.cxx
        Runtime/Source/Memory.cxx

    PUBLIC_INCLUDES Runtime/Include/Public

    PUBLIC_DEPENDENCIES
        glm::glm

)
