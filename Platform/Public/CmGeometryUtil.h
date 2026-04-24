#pragma once

#include "CmGraphicContext.h"

namespace chimi
{
struct CmVertex
{
    glm::vec3 position{};
    glm::vec2 texcoord0{};
    glm::vec3 normal{};
};

class CmGeometryUtil
{
public:
    static void CreateCube(
        float leftPlane,
        float rightPlane,
        float bottomPlane,
        float topPlane,
        float nearPlane,
        float farPlane,
        std::vector<CmVertex>& vertices,
        std::vector<uint32_t>& indices,
        bool useTexcoords = true,
        bool useNormals = true,
        const glm::mat4& relativeMatrix = glm::mat4(1.0f));
};
}
