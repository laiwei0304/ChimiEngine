#pragma once

#include "ECS/CmScene.h"

namespace chimi{
    class CmApplication;
    class CmRenderContext;

    struct CmAppContext{
        CmApplication *app;
        CmScene *scene;
        CmRenderContext *renderCxt;
    };
}
