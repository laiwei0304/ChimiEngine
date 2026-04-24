#pragma once

#include "Graphic/CmVKGraphicContext.h"
#include "Graphic/CmVkSwapchain.h"
#include "Graphic/CmVKDevice.h"

namespace chimi{
    class CmWindow;

    class CmRenderContext{
    public:
        CmRenderContext(CmWindow *window);
        ~CmRenderContext();

        CmGraphicContext *GetGraphicContext() const { return mGraphicContext.get(); }
        CmVKDevice *GetDevice() const { return mDevice.get(); }
        CmVKSwapchain *GetSwapchain() const { return mSwapchain.get(); }
    private:
        std::shared_ptr<CmGraphicContext> mGraphicContext;
        std::shared_ptr<CmVKDevice> mDevice;
        std::shared_ptr<CmVKSwapchain> mSwapchain;
    };
}
