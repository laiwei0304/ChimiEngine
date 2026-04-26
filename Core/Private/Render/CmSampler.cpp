#include "Render/CmSampler.h"

#include "CmApplication.h"
#include "Render/CmRenderContext.h"

namespace chimi{
    CmSampler::CmSampler(VkFilter filter, VkSamplerAddressMode addressMode) : mFilter(filter), mAddressMode(addressMode) {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();

        CALL_VK(device->CreateSimpleSampler(mFilter, mAddressMode, &mHandle));
    }

    CmSampler::~CmSampler() {
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();
        VK_D(Sampler, device->GetHandle(), mHandle);
    }
}