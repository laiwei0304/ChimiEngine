#pragma once

#include "CmVKCommon.h"

namespace chimi
{
    class CmVKDevice;

    class CmVKBuffer
    {
        public:
            CmVKBuffer(CmVKDevice *device, VkBufferUsageFlags usage, size_t size, void *data = nullptr, bool bHostVisible = false);
            ~CmVKBuffer();

            static void CreateBufferInternal(CmVKDevice *device, VkMemoryPropertyFlags memProps, VkBufferUsageFlags usage, size_t size, VkBuffer *outBuffer, VkDeviceMemory *outMemory);
            static void CopyToBuffer(CmVKDevice *device, VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size);

            VkResult WriteData(void *data);

            VkBuffer GetHandle() const { return mHandle; }
        private:
            void CreateBuffer(VkBufferUsageFlags usage, void *data);

            VkBuffer mHandle = VK_NULL_HANDLE;
            VkDeviceMemory mMemory = VK_NULL_HANDLE;

            CmVKDevice *mDevice;
            size_t mSize;
            bool bHostVisible;
    };
}
