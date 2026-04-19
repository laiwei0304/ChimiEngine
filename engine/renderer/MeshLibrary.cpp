#include "renderer/MeshLibrary.h"

#include "core/Assert.h"

namespace chimi::renderer
{
void MeshLibrary::RegisterMesh(uint64_t meshId, CpuMeshData meshData)
{
    CHIMI_ASSERT(meshId != 0, "MeshLibrary requires non-zero meshId");

    for (Entry& entry : m_entries)
    {
        if (entry.meshId == meshId)
        {
            entry.meshData = std::move(meshData);
            return;
        }
    }

    m_entries.push_back(Entry{
        .meshId = meshId,
        .meshData = std::move(meshData)
    });
}

const CpuMeshData* MeshLibrary::FindMesh(uint64_t meshId) const
{
    for (const Entry& entry : m_entries)
    {
        if (entry.meshId == meshId)
        {
            return &entry.meshData;
        }
    }

    return nullptr;
}
}
