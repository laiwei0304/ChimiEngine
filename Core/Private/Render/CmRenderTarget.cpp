#include "Render/CmRenderTarget.h"
#include "CmApplication.h"
#include "Graphic/CmVKRenderPass.h"
#include "Graphic/CmVKImage.h"
#include "ECS/Component/CmLookAtCameraComponent.h"

namespace chimi{
    CmRenderTarget::CmRenderTarget(CmVKRenderPass *renderPass) {
        CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        mRenderPass = renderPass;
        mBufferCount = swapchain->GetImages().size();
        mExtent = { swapchain->GetWidth(), swapchain->GetHeight() };
        bSwapchainTarget = true;

        Init();
        ReCreate();
    }

    CmRenderTarget::CmRenderTarget(CmVKRenderPass *renderPass, uint32_t bufferCount, VkExtent2D extent) :
                        mRenderPass(renderPass), mBufferCount(bufferCount), mExtent(extent), bSwapchainTarget(false) {
        Init();
        ReCreate();
    }

    CmRenderTarget::~CmRenderTarget() {
        for (const auto &item: mMaterialSystemList){
            item->OnDestroy();
        }
        mMaterialSystemList.clear();
    }

    void CmRenderTarget::Init() {
        mClearValues.resize(mRenderPass->GetAttachmentSize());
        SetColorClearValue({ 0.f, 0.f, 0.f, 1.f });
        SetDepthStencilClearValue({ 1.f, 0 });
    }

    void CmRenderTarget::ReCreate() {
        if(mExtent.width == 0 || mExtent.height == 0){
            return;
        }
        mFrameBuffers.clear();
        mFrameBuffers.resize(mBufferCount);

        CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        CmVKDevice *device = renderCxt->GetDevice();
        CmVKSwapchain *swapchain = renderCxt->GetSwapchain();

        std::vector<Attachment> attachments = mRenderPass->GetAttachments();
        if(attachments.empty()){
            return;
        }

        std::vector<VkImage> swapchainImages = swapchain->GetImages();

        for(int i = 0; i < mBufferCount; i++){
            std::vector<std::shared_ptr<CmVKImage>> images;
            for(int j = 0; j < attachments.size(); j++){
                Attachment attachment = attachments[j];
                if(bSwapchainTarget && attachment.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && attachment.samples == VK_SAMPLE_COUNT_1_BIT){
                    images.push_back(std::make_shared<CmVKImage>(device, swapchainImages[i], VkExtent3D{ mExtent.width, mExtent.height, 1 }, attachment.format, attachment.usage));
                } else {
                    images.push_back(std::make_shared<CmVKImage>(device, VkExtent3D{ mExtent.width, mExtent.height, 1 }, attachment.format, attachment.usage, attachment.samples));
                }
            }
            mFrameBuffers[i] = std::make_shared<CmVKFrameBuffer>(device, mRenderPass, images, mExtent.width, mExtent.height);
            images.clear();
        }
    }

    void CmRenderTarget::Begin(VkCommandBuffer cmdBuffer) {
        assert(!bBeginTarget && "You should not called Begin() again.");

        if(bShouldUpdate){
            ReCreate();
            bShouldUpdate = false;
        }
        if(CmEntity::HasComponent<CmLookAtCameraComponent>(mCamera)){
            mCamera->GetComponent<CmLookAtCameraComponent>().SetAspect(mExtent.width * 1.f / mExtent.height);
        }
        if(bSwapchainTarget){
            CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
            CmVKSwapchain *swapchain = renderCxt->GetSwapchain();
            mCurrentBufferIdx = swapchain->GetCurrentImageIndex();
        } else {
            mCurrentBufferIdx = (mCurrentBufferIdx + 1) % mBufferCount;
        }

        mRenderPass->Begin(cmdBuffer, GetFrameBuffer(), mClearValues);
        bBeginTarget = true;
    }

    void CmRenderTarget::End(VkCommandBuffer cmdBuffer) {
        if(bBeginTarget){
            mRenderPass->End(cmdBuffer);
            bBeginTarget = false;
        }
    }

    void CmRenderTarget::SetExtent(const VkExtent2D &extent) {
        mExtent = extent;
        bShouldUpdate = true;
    }

    void CmRenderTarget::SetBufferCount(uint32_t bufferCount) {
        mBufferCount = bufferCount;
        bShouldUpdate = true;
    }

    void CmRenderTarget::SetColorClearValue(VkClearColorValue colorClearValue) {
        std::vector<Attachment> renderPassAttachments = mRenderPass->GetAttachments();
        for(int i = 0; i < renderPassAttachments.size(); i++){
            if(!IsDepthStencilFormat(renderPassAttachments[i].format) && renderPassAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR){
                mClearValues[i].color = colorClearValue;
            }
        }
    }

    void CmRenderTarget::SetDepthStencilClearValue(VkClearDepthStencilValue depthStencilValue) {
        std::vector<Attachment> renderPassAttachments = mRenderPass->GetAttachments();
        for(int i = 0; i < renderPassAttachments.size(); i++){
            if(IsDepthStencilFormat(renderPassAttachments[i].format) && renderPassAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR){
                mClearValues[i].depthStencil = depthStencilValue;
            }
        }
    }

    void CmRenderTarget::SetColorClearValue(uint32_t attachmentIndex, VkClearColorValue colorClearValue) {
        std::vector<Attachment> renderPassAttachments = mRenderPass->GetAttachments();
        if(attachmentIndex <= renderPassAttachments.size() - 1){
            if(!IsDepthStencilFormat(renderPassAttachments[attachmentIndex].format) && renderPassAttachments[attachmentIndex].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR){
                mClearValues[attachmentIndex].color = colorClearValue;
            }
        }
    }

    void CmRenderTarget::SetDepthStencilClearValue(uint32_t attachmentIndex, VkClearDepthStencilValue depthStencilValue) {
        std::vector<Attachment> renderPassAttachments = mRenderPass->GetAttachments();
        if(attachmentIndex <= renderPassAttachments.size() - 1){
            if(IsDepthStencilFormat(renderPassAttachments[attachmentIndex].format) && renderPassAttachments[attachmentIndex].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR){
                mClearValues[attachmentIndex].depthStencil = depthStencilValue;
            }
        }
    }
}