#include "Render/CmTexture.h"

#include "CmApplication.h"
#include "Render/CmRenderContext.h"
#include "Graphic/CmVKDevice.h"
#include "Graphic/CmVKImage.h"
#include "Graphic/CmVKImageView.h"
#include "Graphic/CmVKBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace chimi{
    CmTexture::CmTexture(const std::string &filePath) {
        int numChannel;
        uint8_t *data = stbi_load(filePath.c_str(), reinterpret_cast<int *>(&mWidth), reinterpret_cast<int *>(&mHeight), &numChannel, STBI_rgb_alpha);
        if(!data){
            LOG_E("Can not load this image: {0}", filePath);
            return;
        }

        mFormat = VK_FORMAT_R8G8B8A8_UNORM;
        size_t size = sizeof(uint8_t) * 4 * mWidth * mHeight;
        CreateImage(size, data);

        stbi_image_free(data);
    }

    CmTexture::CmTexture(uint32_t width, uint32_t height, RGBAColor *pixels) : mWidth(width), mHeight(height) {
        mFormat = VK_FORMAT_R8G8B8A8_UNORM;
        size_t size = sizeof(uint8_t) * 4 * mWidth * mHeight;
        CreateImage(size, pixels);
    }

    CmTexture::~CmTexture() {
        mImageView.reset();
        mImage.reset();
    }

    void CmTexture::CreateImage(size_t size, void *data) {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        mImage = std::make_shared<CmVKImage>(device, VkExtent3D{ mWidth, mHeight, 1}, mFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT);
        mImageView = std::make_shared<CmVKImageView>(device, mImage->GetHandle(), mFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        // copy data to buffer
        std::shared_ptr<CmVKBuffer> stageBuffer = std::make_shared<CmVKBuffer>(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, data, true);

        // copy buffer to image
        VkCommandBuffer cmdBuffer = device->CreateAndBeginOneCmdBuffer();
        CmVKImage::TransitionLayout(cmdBuffer, mImage->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        mImage->CopyFromBuffer(cmdBuffer, stageBuffer.get());
        CmVKImage::TransitionLayout(cmdBuffer, mImage->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        device->SubmitOneCmdBuffer(cmdBuffer);
        stageBuffer.reset();
    }
}
