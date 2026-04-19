#include "platform/Window.h"

#include "core/Assert.h"

#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

namespace
{
void GLFWErrorCallback(int error, const char* description)
{
    spdlog::error("GLFW error {}: {}", error, description);
}
}

namespace chimi::platform
{
Window::Window(int width, int height, std::string_view title)
{
    glfwSetErrorCallback(GLFWErrorCallback);
    CHIMI_ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");

    CHIMI_ASSERT(glfwVulkanSupported() == GLFW_TRUE, "GLFW reports Vulkan is unavailable");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    CHIMI_ASSERT(m_window != nullptr, "Failed to create GLFW window");

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, GLFWFramebufferSizeCallback);
}

Window::~Window()
{
    if (m_window != nullptr)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
}

void Window::PollEvents() const
{
    glfwPollEvents();
}

bool Window::ShouldClose() const
{
    return m_window == nullptr || glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

GLFWwindow* Window::GetNativeHandle() const
{
    return m_window;
}

VkSurfaceKHR Window::CreateVulkanSurface(VkInstance instance) const
{
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VK_CHECK(glfwCreateWindowSurface(instance, m_window, nullptr, &surface));
    return surface;
}

VkExtent2D Window::GetFramebufferExtent() const
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);

    VkExtent2D extent{};
    extent.width = static_cast<uint32_t>(width);
    extent.height = static_cast<uint32_t>(height);
    return extent;
}

bool Window::WasFramebufferResized() const
{
    return m_framebufferResized;
}

void Window::ResetFramebufferResizedFlag()
{
    m_framebufferResized = false;
}

void Window::GLFWFramebufferSizeCallback(GLFWwindow* window, int, int)
{
    auto* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner != nullptr)
    {
        owner->m_framebufferResized = true;
    }
}
}
