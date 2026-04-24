#pragma once

#include "Graphic/CmVKCommon.h"

namespace chimi{
    class CmSampler{
    public:
        CmSampler(VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
        ~CmSampler();

        VkSampler GetHandle() const { return mHandle; }
    private:
        VkSampler mHandle = VK_NULL_HANDLE;

        VkFilter mFilter;
        VkSamplerAddressMode mAddressMode;
    };
}
