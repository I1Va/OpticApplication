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

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/gui-interface)

add_library(AIDr4Plugin SHARED IMPORTED)
set_target_properties(AIDr4Plugin PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/libAIPlugin.so"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cum
    dr4
    hui
    pp
    AIDr4Plugin
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/gui-interface/include
)
    

