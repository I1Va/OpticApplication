cmake_minimum_required(VERSION 3.20)
project(OpticApplication2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

option(SANITIZE "Enable compiler sanitizers" OFF)
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
    add_link_options(
         $<$<BOOL:${SANITIZE}>:-fsanitize=address,undefined>
    )
endif()

set(GEOMLIB_BUILD_TESTS OFF CACHE BOOL "Disable GeomLib tests")
add_subdirectory(external/GeomLib)
add_subdirectory(external/RayTracer)

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/source/ROACommon.cpp    
    ${CMAKE_CURRENT_SOURCE_DIR}/source/SVGParser/SVGImageConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/gui-interface)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cum
    dr4
    hui
    pp
    GeomLib
    RayTracer
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/gui-interface/include
    ${CMAKE_SOURCE_DIR}/external/RayTracer/inc
    ${CMAKE_SOURCE_DIR}/external/GeomLib/inc
    ${CMAKE_SOURCE_DIR}/include    
)

    

