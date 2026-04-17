#pragma once

#include <string_view>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace chimi::platform
{
class Window
{
public:
    Window(int width, int height, std::string_view title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void PollEvents() const;
    bool ShouldClose() const;

    GLFWwindow* GetNativeHandle() const;
    VkSurfaceKHR CreateVulkanSurface(VkInstance instance) const;
    VkExtent2D GetFramebufferExtent() const;
    bool WasFramebufferResized() const;
    void ResetFramebufferResizedFlag();

private:
    static void GLFWFramebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window = nullptr;
    bool m_framebufferResized = false;
};
}
