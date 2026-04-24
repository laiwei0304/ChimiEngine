#include "Render/CmRenderContext.h"

namespace chimi{
    CmRenderContext::CmRenderContext(CmWindow *window) {
        mGraphicContext = chimi::CmGraphicContext::Create(window);
        auto vkContext = dynamic_cast<chimi::CmVKGraphicContext*>(mGraphicContext.get());
        mDevice = std::make_shared<chimi::CmVKDevice>(vkContext, 1, 1);
        mSwapchain = std::make_shared<chimi::CmVKSwapchain>(vkContext, mDevice.get());
    }

    CmRenderContext::~CmRenderContext() {

    }
}