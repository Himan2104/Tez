cmake_minimum_required(VERSION 3.22)

project(Tez VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS ON)

# Dependencies
add_subdirectory(External/SDL)
add_subdirectory(External/glm)

add_subdirectory(Core)
add_subdirectory(Application)



