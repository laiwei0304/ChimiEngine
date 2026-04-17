#include "platform/Window.h"

#include "core/Assert.h"

#include <GLFW/glfw3.h>

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
}
