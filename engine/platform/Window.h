#pragma once

#include <string_view>

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

private:
    GLFWwindow* m_window = nullptr;
};
}
