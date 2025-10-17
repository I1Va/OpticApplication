cmake_minimum_required(VERSION 3.10)
project(OpticApplication LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(SDL2_GFX_LIB SDL2_gfx)
find_path(SDL2_GFX_INCLUDE SDL2/SDL2_gfxPrimitives.h)

add_subdirectory(libs/GeomLib)
add_subdirectory(libs/MyGUI)
add_subdirectory(libs/RayTracer)
add_subdirectory(libs/geometry_module)

add_executable(OpticApplication
    main.cpp
)

target_include_directories(OpticApplication
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_include_directories(OpticApplication PRIVATE ${SDL2_GFX_INCLUDE})
target_link_libraries(OpticApplication PRIVATE ${SDL2_GFX_LIB})

target_link_libraries(OpticApplication PRIVATE
    GeomLib
    MyGUI
    RayTracer
    geometry_module
)   