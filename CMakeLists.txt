cmake_minimum_required(VERSION 3.25)

project(ChimiEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Vulkan REQUIRED)

set(CHIMI_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated/shaders")
set(CHIMI_RESOURCE_ROOT_DIR "${CMAKE_SOURCE_DIR}/Resource/")

if(WIN32)
    add_compile_definitions(
        CHIMI_ENGINE_PLATFORM_WIN32
        VK_USE_PLATFORM_WIN32_KHR
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
elseif(APPLE)
    add_compile_definitions(
        CHIMI_ENGINE_PLATFORM_MACOS
        VK_USE_PLATFORM_MACOS_MVK
    )
elseif(UNIX)
    add_compile_definitions(
        CHIMI_ENGINE_PLATFORM_LINUX
        VK_USE_PLATFORM_XCB_KHR
    )
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_compile_definitions(
    CHIMI_RES_ROOT_DIR="${CHIMI_RESOURCE_ROOT_DIR}"
    CHIMI_SHADER_OUTPUT_DIR="${CHIMI_SHADER_OUTPUT_DIR}"
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    CHIMI_ENABLE_VALIDATION=$<IF:$<CONFIG:Debug>,1,0>
)

include(cmake/Dependencies.cmake)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(Platform)
add_subdirectory(Core)
add_subdirectory(Editor)
add_subdirectory(Sample)
