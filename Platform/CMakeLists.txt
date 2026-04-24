add_library(chimi_platform
    Private/CmLog.cpp
    Private/CmGeometryUtil.cpp
    Private/CmWindow.cpp
    Private/CmGraphicContext.cpp
    Private/Window/CmGLFWwindow.cpp
    Private/Event/CmEventDispatcher.cpp
    Private/Graphic/CmVKGraphicContext.cpp
    Private/Graphic/CmVKDevice.cpp
    Private/Graphic/CmVKSwapchain.cpp
    Private/Graphic/CmVKQueue.cpp
    Private/Graphic/CmVKBuffer.cpp
    Private/Graphic/CmVKImage.cpp
    Private/Graphic/CmVKImageView.cpp
    Private/Graphic/CmVKRenderPass.cpp
    Private/Graphic/CmVKFrameBuffer.cpp
    Private/Graphic/CmVKPipeline.cpp
    Private/Graphic/CmVKCommandBuffer.cpp
    Private/Graphic/CmVKDescriptorSet.cpp
)

target_include_directories(chimi_platform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_include_directories(chimi_platform PUBLIC External)

target_link_libraries(chimi_platform
    PUBLIC
        glfw
        glm::glm
        spdlog::spdlog
        volk
)

if (DEFINED CHIMI_VMA_TARGET)
    target_link_libraries(chimi_platform PUBLIC ${CHIMI_VMA_TARGET})
endif()

target_compile_features(chimi_platform PUBLIC cxx_std_20)

target_compile_definitions(chimi_platform
    PUBLIC
        GLFW_INCLUDE_NONE
        VK_NO_PROTOTYPES
)

if (MSVC)
    target_compile_options(chimi_platform PRIVATE /W4 /permissive-)
else()
    target_compile_options(chimi_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()
