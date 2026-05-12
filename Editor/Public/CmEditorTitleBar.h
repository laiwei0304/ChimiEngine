#pragma once

#include "CmEngine.h"
#include "glm/glm.hpp"

namespace chimi{
    class CmEditorContext;

    class CmEditorTitleBar{
    public:
        void Init(CmEditorContext *context, bool *showStats, bool *showContent, bool *showDemoWindow);
        void OnImGuiRender();
        void Destroy();
    private:
        void DrawMenu();
        void DrawWindowButtons();
        void HandleWindowDrag();
        void HandleWindowResize();
        void ToggleMaximized();

        CmEditorContext *mContext = nullptr;
        bool *bShowStats = nullptr;
        bool *bShowContent = nullptr;
        bool *bShowDemoWindow = nullptr;

        bool bDraggingWindow = false;
        bool bResizingWindow = false;

        glm::vec2 mDragStartMousePos{ 0.f, 0.f };
        glm::ivec2 mDragStartWindowPos{ 0, 0 };
        glm::vec2 mResizeStartMousePos{ 0.f, 0.f };
        glm::ivec2 mResizeStartWindowPos{ 0, 0 };
        glm::ivec2 mResizeStartWindowSize{ 0, 0 };
        uint32_t mResizeEdges = 0;
    };
}
