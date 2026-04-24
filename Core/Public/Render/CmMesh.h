#pragma once

#include "Graphic/CmVKBuffer.h"
#include "CmGeometryUtil.h"

namespace chimi{
    class CmMesh{
    public:
        CmMesh(const std::vector<chimi::CmVertex> &vertices, const std::vector<uint32_t> &indices = {});
        ~CmMesh();

        void Draw(VkCommandBuffer cmdBuffer);

    private:
        std::shared_ptr<CmVKBuffer> mVertexBuffer;
        std::shared_ptr<CmVKBuffer> mIndexBuffer;
        uint32_t mVertexCount;
        uint32_t mIndexCount;
    };
}
