#pragma once

namespace chimi{
    class CmApplication;
    class CmRenderContext;
    class CmScene;

    struct CmAppContext{
        CmApplication *app;
        CmScene *scene;
        CmRenderContext *renderCxt;
    };
}
