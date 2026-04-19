#pragma once

#include "renderer/MeshData.h"

#include <cstdint>
#include <vector>

namespace chimi::renderer
{
class MeshLibrary
{
public:
    void RegisterMesh(uint64_t meshId, CpuMeshData meshData);
    const CpuMeshData* FindMesh(uint64_t meshId) const;

private:
    struct Entry
    {
        uint64_t meshId = 0;
        CpuMeshData meshData{};
    };

    std::vector<Entry> m_entries;
};
}
