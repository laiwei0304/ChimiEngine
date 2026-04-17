#pragma once

#include <cstdint>
#include <vector>

#include <glm/vec3.hpp>

namespace chimi::renderer
{
struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

struct CpuMeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
}
