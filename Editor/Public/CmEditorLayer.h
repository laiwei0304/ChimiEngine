#pragma once

#include "CmEngine.h"
#include "CmEditorTitleBar.h"
#include "Panel/CmPanel.h"

namespace chimi{
    class CmEditorContext;

    class CmEditorLayer{
    public:
        CmEditorLayer() = default;
        ~CmEditorLayer();

        void Init(CmEditorContext *context);
        void OnImGuiRender();
        void Destroy();
    private:
        void DrawDockSpace();
        void DrawViewportPanel();
        void DrawScenePanel();
        void DrawInspectorPanel();
        void DrawContentPanel();
        void DrawStatsPanel();
        void ResetDockLayout(uint32_t dockSpaceId);

        CmEditorContext *mContext = nullptr;
        CmEditorTitleBar mTitleBar;
        bool bShowDemoWindow = false;
        bool bShowStats = true;
        bool bShowContent = true;
        bool bDockLayoutInitialized = false;
    };
}
