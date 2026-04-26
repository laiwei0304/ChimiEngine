#include "Graphic/CmVKDescriptorSet.h"
#include "Graphic/CmVKDevice.h"

namespace chimi
{
    CmVKDescriptorSetLayout::CmVKDescriptorSetLayout(CmVKDevice *device, const std::vector<VkDescriptorSetLayoutBinding> &bindings) : mDevice(device) {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data()
        };
        CALL_VK(vkCreateDescriptorSetLayout(mDevice->GetHandle(), &descriptorSetLayoutInfo, nullptr, &mHandle));
    }

    CmVKDescriptorSetLayout::~CmVKDescriptorSetLayout() {
        VK_D(DescriptorSetLayout, mDevice->GetHandle(), mHandle);
    }

    CmVKDescriptorPool::CmVKDescriptorPool(CmVKDevice *device, uint32_t maxSets, const std::vector<VkDescriptorPoolSize> &poolSizes) : mDevice(device) {
        VkDescriptorPoolCreateInfo descriptorPoolInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .maxSets = maxSets,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data()
        };
        CALL_VK(vkCreateDescriptorPool(mDevice->GetHandle(), &descriptorPoolInfo, nullptr, &mHandle));
    }

    CmVKDescriptorPool::~CmVKDescriptorPool() {
        VK_D(DescriptorPool, mDevice->GetHandle(), mHandle);
    }

    std::vector<VkDescriptorSet> CmVKDescriptorPool::AllocateDescriptorSet(CmVKDescriptorSetLayout *setLayout, uint32_t count) {
        std::vector<VkDescriptorSet> descriptorSets(count);
        std::vector<VkDescriptorSetLayout> setLayouts(count);
        for(int i = 0; i < count; i++){
            setLayouts[i] = setLayout->GetHandle();
        }
        VkDescriptorSetAllocateInfo allocateInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = mHandle,
                .descriptorSetCount = count,
                .pSetLayouts = setLayouts.data()
        };
        VkResult ret = vkAllocateDescriptorSets(mDevice->GetHandle(), &allocateInfo, descriptorSets.data());
        CALL_VK(ret);
        if(ret != VK_SUCCESS){
            descriptorSets.clear();
        }
        return descriptorSets;
    }
}
