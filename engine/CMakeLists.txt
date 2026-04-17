add_library(ChimiEngine
    core/Assert.cpp
    core/Log.cpp
    platform/Window.cpp
    rhi/vulkan/VulkanInstance.cpp
)

target_include_directories(ChimiEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ChimiEngine
    PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        spdlog::spdlog
        volk
)

if (TARGET ChimiEngine::VMA)
    target_link_libraries(ChimiEngine PUBLIC ChimiEngine::VMA)
endif()

target_compile_features(ChimiEngine PUBLIC cxx_std_20)

target_compile_definitions(ChimiEngine
    PUBLIC
        GLFW_INCLUDE_NONE
        VK_NO_PROTOTYPES
    PRIVATE
        $<$<BOOL:${CHIMI_ENABLE_VALIDATION}>:CHIMI_ENABLE_VALIDATION=1>
        $<$<NOT:$<BOOL:${CHIMI_ENABLE_VALIDATION}>>:CHIMI_ENABLE_VALIDATION=0>
)

if (MSVC)
    target_compile_options(ChimiEngine PRIVATE /W4 /permissive-)
else()
    target_compile_options(ChimiEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
