#include "core/Log.h"
#include "platform/Window.h"
#include "rhi/vulkan/VulkanInstance.h"

#include <exception>

#include "spdlog/spdlog.h"

int main()
{
    chimi::core::InitializeLogging();

    try
    {
        chimi::platform::Window window(1280, 720, "Chimi Engine Sandbox");
        chimi::rhi::vulkan::VulkanInstance instance;

        spdlog::info("Sandbox started successfully");

        while (!window.ShouldClose())
        {
            window.PollEvents();
        }

        spdlog::info("Sandbox exiting");
        (void)instance;
        return 0;
    }
    catch (const std::exception& exception)
    {
        spdlog::critical("Unhandled exception: {}", exception.what());
        return 1;
    }
}
