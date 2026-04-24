#pragma once

#include "CmWindow.h"
#include <GLFW/glfw3.h>

namespace chimi
{
    class CmGLFWwindow final : public CmWindow
    {
        public:
            CmGLFWwindow() = delete;
            CmGLFWwindow(uint32_t width, uint32_t height, const char *title);
            ~CmGLFWwindow() override;

            bool ShouldClose() override;
            void PollEvents() override;
            void SwapBuffer() override;

            void* GetImplWindowPointer() const override { return mGLFWwindow; };

            void GetMousePos(glm::vec2 &mousePos) const override;
            bool IsMouseDown(MouseButton mouseButton) const override;
            bool IsMouseUp(MouseButton mouseButton) const override;
            bool IsKeyDown(Key key) const override;
            bool IsKeyUp(Key key) const override;
        private:
            void SetupWindowCallbacks();

            GLFWwindow *mGLFWwindow;
    };
}
