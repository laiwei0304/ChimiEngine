#pragma once

#include "CmMaterialComponent.h"

namespace chimi{
    enum BaseMaterialColor{
        COLOR_TYPE_NORMAL = 0,
        COLOR_TYPE_TEXCOORD = 1
    };

    struct CmBaseMaterial : public CmMaterial{
        BaseMaterialColor colorType;
    };

    struct CmBaseMaterialComponent : public CmMaterialComponent<CmBaseMaterial> {

    };
}
