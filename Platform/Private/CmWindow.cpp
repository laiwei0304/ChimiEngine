#include "CmWindow.h"
#include "Window/CmGLFWwindow.h"

namespace chimi
{
std::unique_ptr<CmWindow> CmWindow::Create(uint32_t width, uint32_t height, const char *title)
{
#if defined(CHIMI_ENGINE_PLATFORM_WIN32) || defined(CHIMI_ENGINE_PLATFORM_MACOS) || defined(CHIMI_ENGINE_PLATFORM_LINUX)
    return std::make_unique<CmGLFWwindow>(width, height, title);
#endif
    return nullptr;
}
}
