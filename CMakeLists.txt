cmake_minimum_required(VERSION 3.20)
project(OpticApplication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

option(GEOMLIB_BUILD_TESTS "GeomLib unit tests" OFF)

add_subdirectory(libs/GeomLib)
add_subdirectory(libs/MyGUI)
add_subdirectory(libs/RayTracer)
add_subdirectory(libs/geometry_module)

add_executable(OpticApplication main.cpp)

target_include_directories(OpticApplication PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(OpticApplication PRIVATE
    GeomLib
    MyGUI
    RayTracer
    geometry_module
)   
