#pragma once

#include "CmEngine.h"

namespace chimi{
    class CmApplication;
    class CmScene;
    class CmEntity;

    enum class CmEditorMode{
        Edit,
        Play
    };

    class CmEditorContext{
    public:
        void SetApp(CmApplication *app) { mApp = app; }
        CmApplication *GetApp() const { return mApp; }

        void SetScene(CmScene *scene);
        CmScene *GetScene() const { return mScene; }

        void SetSelectedEntity(CmEntity *entity) { mSelectedEntity = entity; }
        CmEntity *GetSelectedEntity() const { return mSelectedEntity; }

        void SetViewportSize(uint32_t width, uint32_t height);
        uint32_t GetViewportWidth() const { return mViewportWidth; }
        uint32_t GetViewportHeight() const { return mViewportHeight; }
        void SetViewportTextureId(uint64_t textureId) { mViewportTextureId = textureId; }
        uint64_t GetViewportTextureId() const { return mViewportTextureId; }

        void SetViewportHovered(bool hovered) { bViewportHovered = hovered; }
        bool IsViewportHovered() const { return bViewportHovered; }

        void SetViewportFocused(bool focused) { bViewportFocused = focused; }
        bool IsViewportFocused() const { return bViewportFocused; }

        void SetBlockSceneInput(bool blockInput) { bBlockSceneInput = blockInput; }
        bool ShouldBlockSceneInput() const { return bBlockSceneInput; }

        void SetMode(CmEditorMode mode) { mMode = mode; }
        CmEditorMode GetMode() const { return mMode; }
    private:
        CmApplication *mApp = nullptr;
        CmScene *mScene = nullptr;
        CmEntity *mSelectedEntity = nullptr;

        uint32_t mViewportWidth = 0;
        uint32_t mViewportHeight = 0;
        uint64_t mViewportTextureId = 0;
        bool bViewportHovered = false;
        bool bViewportFocused = false;
        bool bBlockSceneInput = false;

        CmEditorMode mMode = CmEditorMode::Edit;
    };
}
