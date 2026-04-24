#include "Render/CmMesh.h"
#include "Render/CmRenderContext.h"
#include "CmApplication.h"

namespace chimi{
    CmMesh::CmMesh(const std::vector<chimi::CmVertex> &vertices, const std::vector<uint32_t> &indices) {
        if(vertices.empty()){
            return;
        }
        chimi::CmRenderContext *renderCxt = CmApplication::GetAppContext()->renderCxt;
        chimi::CmVKDevice *device = renderCxt->GetDevice();

        mVertexCount = vertices.size();
        mIndexCount = indices.size();
        mVertexBuffer = std::make_shared<chimi::CmVKBuffer>(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(vertices[0]) * vertices.size(), (void*)vertices.data());
        if(mIndexCount > 0){
            mIndexBuffer = std::make_shared<chimi::CmVKBuffer>(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(indices[0]) * indices.size(), (void*)indices.data());
        }
    }

    CmMesh::~CmMesh() {

    }

    void CmMesh::Draw(VkCommandBuffer cmdBuffer) {
        VkBuffer vertexBuffers[] = { mVertexBuffer->GetHandle() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

        if(mIndexCount > 0){
            vkCmdBindIndexBuffer(cmdBuffer, mIndexBuffer->GetHandle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmdBuffer, mIndexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(cmdBuffer, mVertexCount, 1, 0, 0);
        }
    }
}