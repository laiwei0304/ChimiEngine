add_library(ChimiEngine
    core/Assert.cpp
    core/Log.cpp
    platform/Window.cpp
    renderer/MeshLibrary.cpp
    renderer/Renderer.cpp
    renderer/SceneExtraction.cpp
    scene/Scene.cpp
    rhi/vulkan/VulkanInstance.cpp
    rhi/vulkan/VulkanContext.cpp
    rhi/vulkan/VulkanMeshBuffer.cpp
    rhi/vulkan/VulkanMeshCache.cpp
    rhi/vulkan/VulkanMeshPass.cpp
    rhi/vulkan/VulkanMeshPipeline.cpp
    rhi/vulkan/VulkanFrameContext.cpp
    rhi/vulkan/VulkanFrameRecording.cpp
    rhi/vulkan/VulkanSwapchain.cpp
    rhi/vulkan/VulkanShaderLoader.cpp
    rhi/vulkan/VulkanUploadContext.cpp
)

target_include_directories(ChimiEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ChimiEngine
    PUBLIC
        glfw
        glm::glm
        spdlog::spdlog
        volk
)

if (DEFINED CHIMI_VMA_TARGET)
    target_link_libraries(ChimiEngine PUBLIC ${CHIMI_VMA_TARGET})
endif()

target_compile_features(ChimiEngine PUBLIC cxx_std_20)

target_compile_definitions(ChimiEngine
    PUBLIC
        GLFW_INCLUDE_NONE
        VK_NO_PROTOTYPES
    PRIVATE
        CHIMI_SHADER_OUTPUT_DIR="${CHIMI_SHADER_OUTPUT_DIR}"
        $<$<BOOL:${CHIMI_ENABLE_VALIDATION}>:CHIMI_ENABLE_VALIDATION=1>
        $<$<NOT:$<BOOL:${CHIMI_ENABLE_VALIDATION}>>:CHIMI_ENABLE_VALIDATION=0>
)

if (MSVC)
    target_compile_options(ChimiEngine PRIVATE /W4 /permissive-)
else()
    target_compile_options(ChimiEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
