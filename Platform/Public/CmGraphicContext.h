#pragma once

#include "CmEngine.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace chimi
{
class CmWindow;
class CmGraphicContext
{
public:
    CmGraphicContext(const CmGraphicContext&) = delete;
    CmGraphicContext& operator=(const CmGraphicContext&) = delete;
    virtual ~CmGraphicContext() = default;

    static std::unique_ptr<CmGraphicContext> Create(CmWindow *window);

protected:
    CmGraphicContext() = default;
};
}
