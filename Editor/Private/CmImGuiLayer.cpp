#include "CmImGuiLayer.h"

#include "CmWindow.h"
#include "Render/CmRenderContext.h"
#include "Graphic/CmVKGraphicContext.h"
#include "Graphic/CmVKQueue.h"
#include "Graphic/CmVKDevice.h"
#include "Graphic/CmVKSwapchain.h"
#include "Graphic/CmVKRenderPass.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include <GLFW/glfw3.h>

namespace chimi{
    namespace{
        void ImGuiCheckVkResult(VkResult result){
            if(result != VK_SUCCESS){
                LOG_E("ImGui Vulkan error: {0}", vk_result_string(result));
            }
        }

        void SetupEditorLightTheme(){
            ImGui::StyleColorsLight();

            ImGuiStyle &style = ImGui::GetStyle();
            style.WindowRounding = 3.f;
            style.ChildRounding = 3.f;
            style.FrameRounding = 3.f;
            style.PopupRounding = 3.f;
            style.ScrollbarRounding = 3.f;
            style.GrabRounding = 3.f;
            style.TabRounding = 3.f;
            style.WindowBorderSize = 1.f;
            style.FrameBorderSize = 1.f;

            ImVec4 *colors = style.Colors;
            colors[ImGuiCol_Text] = ImVec4{ 0.12f, 0.13f, 0.14f, 1.00f };
            colors[ImGuiCol_TextDisabled] = ImVec4{ 0.50f, 0.52f, 0.55f, 1.00f };
            colors[ImGuiCol_WindowBg] = ImVec4{ 0.94f, 0.95f, 0.96f, 1.00f };
            colors[ImGuiCol_ChildBg] = ImVec4{ 0.97f, 0.98f, 0.98f, 1.00f };
            colors[ImGuiCol_PopupBg] = ImVec4{ 1.00f, 1.00f, 1.00f, 0.98f };
            colors[ImGuiCol_Border] = ImVec4{ 0.72f, 0.75f, 0.78f, 1.00f };
            colors[ImGuiCol_FrameBg] = ImVec4{ 1.00f, 1.00f, 1.00f, 1.00f };
            colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.88f, 0.93f, 0.98f, 1.00f };
            colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.78f, 0.87f, 0.96f, 1.00f };
            colors[ImGuiCol_TitleBg] = ImVec4{ 0.88f, 0.90f, 0.92f, 1.00f };
            colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.80f, 0.86f, 0.92f, 1.00f };
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.90f, 0.91f, 0.93f, 1.00f };
            colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.91f, 0.93f, 0.95f, 1.00f };
            colors[ImGuiCol_Header] = ImVec4{ 0.82f, 0.89f, 0.97f, 1.00f };
            colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.74f, 0.84f, 0.95f, 1.00f };
            colors[ImGuiCol_HeaderActive] = ImVec4{ 0.62f, 0.76f, 0.91f, 1.00f };
            colors[ImGuiCol_Button] = ImVec4{ 0.92f, 0.94f, 0.96f, 1.00f };
            colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.82f, 0.89f, 0.97f, 1.00f };
            colors[ImGuiCol_ButtonActive] = ImVec4{ 0.70f, 0.82f, 0.94f, 1.00f };
            colors[ImGuiCol_Tab] = ImVec4{ 0.86f, 0.89f, 0.92f, 1.00f };
            colors[ImGuiCol_TabHovered] = ImVec4{ 0.75f, 0.84f, 0.94f, 1.00f };
            colors[ImGuiCol_TabActive] = ImVec4{ 0.94f, 0.95f, 0.96f, 1.00f };
            colors[ImGuiCol_Separator] = ImVec4{ 0.72f, 0.75f, 0.78f, 1.00f };
            colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.70f, 0.78f, 0.86f, 0.55f };
            colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.56f, 0.70f, 0.86f, 0.75f };
            colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.42f, 0.62f, 0.83f, 0.95f };
            colors[ImGuiCol_CheckMark] = ImVec4{ 0.20f, 0.48f, 0.78f, 1.00f };
            colors[ImGuiCol_SliderGrab] = ImVec4{ 0.45f, 0.63f, 0.82f, 1.00f };
            colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.28f, 0.52f, 0.78f, 1.00f };
        }
    }

    CmImGuiLayer::~CmImGuiLayer() {
        Destroy();
    }

    void CmImGuiLayer::Init(CmWindow *window, CmRenderContext *renderContext, CmVKRenderPass *renderPass) {
        if(bInitialized){
            return;
        }
        assert(window && "Window is null.");
        assert(renderContext && "Render context is null.");
        assert(renderPass && "Render pass is null.");

        auto *vkContext = dynamic_cast<CmVKGraphicContext*>(renderContext->GetGraphicContext());
        CmVKDevice *device = renderContext->GetDevice();
        CmVKSwapchain *swapchain = renderContext->GetSwapchain();
        assert(vkContext && "ImGui layer only supports Vulkan now.");
        assert(device && "Vulkan device is null.");
        assert(swapchain && "Vulkan swapchain is null.");

        IMGUI_CHECKVERSION();
        mContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(mContext);

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 16.f;
        io.Fonts->AddFontDefault(&fontConfig);

        SetupEditorLightTheme();

        GLFWwindow *glfwWindow = static_cast<GLFWwindow*>(window->GetImplWindowPointer());
        ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

        uint32_t imageCount = static_cast<uint32_t>(swapchain->GetImages().size());
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = vkContext->GetInstance();
        initInfo.PhysicalDevice = vkContext->GetPhyDevice();
        initInfo.Device = device->GetHandle();
        initInfo.QueueFamily = static_cast<uint32_t>(vkContext->GetGraphicQueueFamilyInfo().queueFamilyIndex);
        initInfo.Queue = device->GetFirstGraphicQueue()->GetHandle();
        initInfo.PipelineCache = device->GetPipelineCache();
        initInfo.DescriptorPoolSize = 128;
        initInfo.MinImageCount = std::max(2u, imageCount);
        initInfo.ImageCount = imageCount;
        initInfo.PipelineInfoMain.RenderPass = renderPass->GetHandle();
        initInfo.PipelineInfoMain.Subpass = 0;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.CheckVkResultFn = ImGuiCheckVkResult;

        if(!ImGui_ImplVulkan_Init(&initInfo)){
            LOG_E("Failed to initialize ImGui Vulkan backend.");
            return;
        }

        bInitialized = true;
    }

    void CmImGuiLayer::BeginFrame() {
        if(!bInitialized){
            return;
        }
        ImGui::SetCurrentContext(mContext);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void CmImGuiLayer::EndFrame(VkCommandBuffer cmdBuffer) {
        if(!bInitialized){
            return;
        }
        ImGui::SetCurrentContext(mContext);
        ImGui::Render();
        ImDrawData *drawData = ImGui::GetDrawData();
        if(drawData && drawData->DisplaySize.x > 0.f && drawData->DisplaySize.y > 0.f){
            ImGui_ImplVulkan_RenderDrawData(drawData, cmdBuffer);
        }
    }

    void CmImGuiLayer::Destroy() {
        if(!bInitialized){
            return;
        }
        ImGui::SetCurrentContext(mContext);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(mContext);
        mContext = nullptr;
        bInitialized = false;
    }

    uint64_t CmImGuiLayer::AddTexture(VkImageView imageView, VkImageLayout imageLayout) {
        if(!bInitialized || imageView == VK_NULL_HANDLE){
            return 0;
        }
        ImGui::SetCurrentContext(mContext);
        VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(imageView, imageLayout);
        return reinterpret_cast<uint64_t>(descriptorSet);
    }

    void CmImGuiLayer::RemoveTexture(uint64_t textureId) {
        if(!bInitialized || textureId == 0){
            return;
        }
        ImGui::SetCurrentContext(mContext);
        ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureId));
    }

    bool CmImGuiLayer::WantCaptureMouse() const {
        if(!bInitialized){
            return false;
        }
        ImGui::SetCurrentContext(mContext);
        return ImGui::GetIO().WantCaptureMouse;
    }

    bool CmImGuiLayer::WantCaptureKeyboard() const {
        if(!bInitialized){
            return false;
        }
        ImGui::SetCurrentContext(mContext);
        return ImGui::GetIO().WantCaptureKeyboard;
    }
}
