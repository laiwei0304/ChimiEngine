#pragma once

#include "CmWindow.h"
#include <GLFW/glfw3.h>

namespace chimi
{
    class CmGLFWwindow final : public CmWindow
    {
        public:
            CmGLFWwindow() = delete;
            CmGLFWwindow(uint32_t width, uint32_t height, const char *title, bool decorated = true);
            ~CmGLFWwindow() override;

            bool ShouldClose() override;
            void SetShouldClose(bool shouldClose) override;
            void PollEvents() override;
            void SwapBuffer() override;

            void* GetImplWindowPointer() const override { return mGLFWwindow; };

            void GetPosition(glm::ivec2 &position) const override;
            void SetPosition(const glm::ivec2 &position) override;
            void GetSize(glm::ivec2 &size) const override;
            void SetSize(const glm::ivec2 &size) override;
            void Minimize() override;
            void Maximize() override;
            void Restore() override;
            bool IsMaximized() const override;

            void GetMousePos(glm::vec2 &mousePos) const override;
            bool IsMouseDown(MouseButton mouseButton) const override;
            bool IsMouseUp(MouseButton mouseButton) const override;
            bool IsKeyDown(Key key) const override;
            bool IsKeyUp(Key key) const override;
        private:
            void SetupWindowCallbacks();

            GLFWwindow *mGLFWwindow = nullptr;
            bool bGLFWInitialized = false;
    };
}
