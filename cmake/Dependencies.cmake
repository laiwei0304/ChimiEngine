include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
)

FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
)

FetchContent_Declare(
    spdlog
    URL https://github.com/gabime/spdlog/archive/refs/tags/v1.14.1.zip
)

FetchContent_Declare(
    volk
    URL https://github.com/zeux/volk/archive/refs/tags/1.4.304.zip
)

FetchContent_Declare(
    vma
    URL https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v3.2.1.zip
)

FetchContent_Declare(
    tinygltf
    URL https://github.com/syoyo/tinygltf/archive/refs/tags/v2.9.7.zip
)

FetchContent_Declare(
    entt
    URL https://github.com/skypjack/entt/archive/refs/tags/v3.15.0.zip
)

FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/31c1ad37456438565541f4919958214b6e762fb4.zip
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VOLK_PULL_IN_VULKAN OFF CACHE BOOL "" FORCE)
set(VOLK_INSTALL OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL_VENDOR OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw glm spdlog volk vma tinygltf entt stb)

set(CHIMI_TINYGLTF_INCLUDE_DIR "${tinygltf_SOURCE_DIR}" CACHE INTERNAL "")

if (NOT TARGET chimi_entt)
    add_library(chimi_entt INTERFACE)
    target_include_directories(chimi_entt INTERFACE "${entt_SOURCE_DIR}/src")
endif()

if (NOT TARGET chimi_stb)
    add_library(chimi_stb INTERFACE)
    target_include_directories(chimi_stb INTERFACE "${stb_SOURCE_DIR}")
endif()

if (TARGET volk)
    target_include_directories(volk PUBLIC ${Vulkan_INCLUDE_DIRS})
endif()

if (TARGET GPUOpen::VulkanMemoryAllocator)
    set(CHIMI_VMA_TARGET GPUOpen::VulkanMemoryAllocator CACHE INTERNAL "")
elseif (TARGET VulkanMemoryAllocator)
    set(CHIMI_VMA_TARGET VulkanMemoryAllocator CACHE INTERNAL "")
endif()
