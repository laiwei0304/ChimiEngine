#include "SandboxMeshes.h"
#include "SandboxScene.h"
#include "SandboxSceneController.h"

#include "core/Log.h"
#include "platform/Window.h"
#include "renderer/MeshLibrary.h"
#include "renderer/Renderer.h"
#include "rhi/vulkan/VulkanInstance.h"
#include "scene/Scene.h"

#include <chrono>
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
        chimi::renderer::MeshLibrary meshLibrary{};
        meshLibrary.RegisterMesh(chimi::app::sandbox::kSampleCubeMeshId, chimi::app::sandbox::CreateSampleCubeMesh());

        chimi::app::sandbox::SandboxSceneBundle sandboxScene =
            chimi::app::sandbox::CreateSandboxSampleScene(chimi::app::sandbox::kSampleCubeMeshId);
        chimi::renderer::Renderer renderer(meshLibrary);
        chimi::rhi::vulkan::VulkanInstance instance(window);
        const auto startupTime = std::chrono::steady_clock::now();

        spdlog::info("Sandbox started successfully");
        spdlog::info("The window should remain open until you close it manually");

        while (!window.ShouldClose())
        {
            window.PollEvents();
            const auto now = std::chrono::steady_clock::now();
            const float elapsedSeconds =
                std::chrono::duration<float>(now - startupTime).count();
            const VkExtent2D framebufferExtent = window.GetFramebufferExtent();
            const float aspectRatio = framebufferExtent.height > 0
                ? static_cast<float>(framebufferExtent.width) / static_cast<float>(framebufferExtent.height)
                : 16.0f / 9.0f;

            chimi::app::sandbox::UpdateSandboxScene(
                sandboxScene,
                elapsedSeconds,
                aspectRatio);

            const chimi::renderer::RenderPacket renderPacket = renderer.BuildRenderPacket(sandboxScene.scene);
            if (window.WasFramebufferResized())
            {
                window.ResetFramebufferResizedFlag();
                instance.HandleResize();
            }
            instance.DrawFrame(renderPacket);
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
