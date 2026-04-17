cmake_minimum_required(VERSION 3.25)

project(ChimiEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(CHIMI_BUILD_SANDBOX "Build the sandbox application" ON)
option(CHIMI_ENABLE_VALIDATION "Enable Vulkan validation layers in debug builds" ON)

find_package(Vulkan REQUIRED)

set(CHIMI_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated/shaders")

include(cmake/Dependencies.cmake)

add_subdirectory(engine)

if (CHIMI_BUILD_SANDBOX)
    add_subdirectory(app/sandbox)
endif()
