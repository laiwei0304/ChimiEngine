#include "CmApplication.h"
#include "CmLog.h"
#include "Render/CmRenderContext.h"
#include "ECS/CmEntity.h"

namespace chimi{
    CmAppContext CmApplication::sAppContext{};

    void CmApplication::Start(int argc, char **argv) {
        CmLog::Init();

        ParseArgs(argc, argv);
        OnConfiguration(&mAppSettings);

        mWindow = CmWindow::Create(mAppSettings.width, mAppSettings.height, mAppSettings.title);
        mRenderContext = std::make_shared<CmRenderContext>(mWindow.get());

        sAppContext.app = this;
        sAppContext.renderCxt = mRenderContext.get();

        OnInit();
        LoadScene();

        mStartTimePoint = std::chrono::steady_clock::now();
    }

    void CmApplication::Stop() {
        UnLoadScene();
        OnDestroy();
    }

    void CmApplication::MainLoop() {
        mLastTimePoint = std::chrono::steady_clock::now();
        while (!mWindow->ShouldClose()) {
            mWindow->PollEvents();

            float deltaTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - mLastTimePoint).count();
            mLastTimePoint = std::chrono::steady_clock::now();
            mFrameIndex++;

            if(!bPause){
                OnUpdate(deltaTime);
            }
            OnRender();

            mWindow->SwapBuffer();
        }
    }

    void CmApplication::ParseArgs(int argc, char **argv) {
        // TODO
    }

    bool CmApplication::LoadScene(const std::string &filePath) {
        if(mScene){
            UnLoadScene();
        }
        mScene = std::make_unique<CmScene>();
        OnSceneInit(mScene.get());
        sAppContext.scene = mScene.get();
        return true;
    }

    void CmApplication::UnLoadScene() {
        if(mScene){
            OnSceneDestroy(mScene.get());
            mScene.reset();
            sAppContext.scene = nullptr;
        }
    }
}