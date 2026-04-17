#include "core/Log.h"
#include "platform/Window.h"
#include "rhi/vulkan/VulkanInstance.h"

#include <exception>

#include <Windows.h>

#include "spdlog/spdlog.h"

namespace
{
LONG WINAPI SandboxUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers)
{
    const DWORD exceptionCode = exceptionPointers != nullptr && exceptionPointers->ExceptionRecord != nullptr
        ? exceptionPointers->ExceptionRecord->ExceptionCode
        : 0;

    const std::string message = "Unhandled structured exception. Code: " + std::to_string(exceptionCode);
    MessageBoxA(
        nullptr,
        message.c_str(),
        "Chimi Engine Sandbox crashed",
        MB_OK | MB_ICONERROR
    );

    return EXCEPTION_EXECUTE_HANDLER;
}

int RunSandbox()
{
    chimi::core::InitializeLogging();

    try
    {
        chimi::platform::Window window(1280, 720, "Chimi Engine Sandbox");
        chimi::rhi::vulkan::VulkanInstance instance(window);

        spdlog::info("Sandbox started successfully");
        spdlog::info("The window should remain open until you close it manually");

        while (!window.ShouldClose())
        {
            window.PollEvents();
            if (window.WasFramebufferResized())
            {
                window.ResetFramebufferResizedFlag();
                instance.HandleResize();
            }
            instance.DrawFrame();
        }

        spdlog::info("Sandbox exiting");
        (void)instance;
        return 0;
    }
    catch (const std::exception& exception)
    {
        spdlog::critical("Unhandled exception: {}", exception.what());
        MessageBoxA(
            nullptr,
            exception.what(),
            "Chimi Engine Sandbox startup failed",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }
}
}

int main()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(SandboxUnhandledExceptionFilter);
    return RunSandbox();
}
