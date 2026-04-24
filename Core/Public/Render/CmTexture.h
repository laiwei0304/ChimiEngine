#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi{
    class CmVKImage;
    class CmVKImageView;
    class CmVKBuffer;

    struct RGBAColor{
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    class CmTexture{
    public:
        CmTexture(const std::string &filePath);
        CmTexture(uint32_t width, uint32_t height, RGBAColor *pixels);
        ~CmTexture();

        uint32_t GetWidth() const { return mWidth; }
        uint32_t GetHeight() const { return mHeight; }
        CmVKImage *GetImage() const { return mImage.get(); }
        CmVKImageView *GetImageView() const { return mImageView.get(); }
    private:
        void CreateImage(size_t size, void *data);

        uint32_t mWidth;
        uint32_t mHeight;
        VkFormat mFormat;
        std::shared_ptr<CmVKImage> mImage;
        std::shared_ptr<CmVKImageView> mImageView;
    };
}
