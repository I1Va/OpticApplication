cmake_minimum_required(VERSION 3.20)
project(OpticApplication LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

option(GEOMLIB_BUILD_TESTS "GeomLib unit tests" OFF)

option(SANITIZE "Enable compiler sanitizers" OFF)
option(BUILD_TESTS "Build unit tests" ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
    add_link_options(
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
endif()

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
