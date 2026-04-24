#include "CmGraphicContext.h"
#include "Graphic/CmVKGraphicContext.h"

namespace chimi
{
std::unique_ptr<CmGraphicContext> CmGraphicContext::Create(CmWindow *window)
{
#ifdef CHIMI_ENGINE_GRAPHIC_API_VULKAN
    return std::make_unique<CmVKGraphicContext>(window);
#else
    return nullptr;
#endif
    return nullptr;
}
}
