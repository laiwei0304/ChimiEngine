#pragma once

#include "Graphic/CmVKCommon.h"

struct ImGuiContext;

namespace chimi{
    class CmWindow;
    class CmRenderContext;
    class CmVKRenderPass;

    class CmImGuiLayer{
    public:
        CmImGuiLayer() = default;
        ~CmImGuiLayer();

        CmImGuiLayer(const CmImGuiLayer&) = delete;
        CmImGuiLayer &operator=(const CmImGuiLayer&) = delete;

        void Init(CmWindow *window, CmRenderContext *renderContext, CmVKRenderPass *renderPass);
        void BeginFrame();
        void EndFrame(VkCommandBuffer cmdBuffer);
        void Destroy();

        uint64_t AddTexture(VkImageView imageView, VkImageLayout imageLayout);
        void RemoveTexture(uint64_t textureId);

        bool IsInitialized() const { return bInitialized; }
        bool WantCaptureMouse() const;
        bool WantCaptureKeyboard() const;
    private:
        ImGuiContext *mContext = nullptr;
        bool bInitialized = false;
    };
}
