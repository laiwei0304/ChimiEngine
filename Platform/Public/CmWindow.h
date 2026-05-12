#pragma once

#include "CmEngine.h"
#include "CmGraphicContext.h"
#include "Event/CmMouseEvent.h"

namespace chimi
{
    class CmWindow
    {
        public:
            CmWindow(const CmWindow&) = delete;
            CmWindow& operator=(const CmWindow&) = delete;
            virtual ~CmWindow() = default;

            static std::unique_ptr<CmWindow> Create(uint32_t width, uint32_t height, const char *title, bool decorated = true);

            virtual bool ShouldClose() = 0;
            virtual void SetShouldClose(bool shouldClose) = 0;
            virtual void PollEvents() = 0;
            virtual void SwapBuffer() = 0;

            virtual void* GetImplWindowPointer() const = 0;

            virtual void GetPosition(glm::ivec2 &position) const = 0;
            virtual void SetPosition(const glm::ivec2 &position) = 0;
            virtual void GetSize(glm::ivec2 &size) const = 0;
            virtual void SetSize(const glm::ivec2 &size) = 0;
            virtual void Minimize() = 0;
            virtual void Maximize() = 0;
            virtual void Restore() = 0;
            virtual bool IsMaximized() const = 0;

            virtual void GetMousePos(glm::vec2 &mousePos) const = 0;
            virtual bool IsMouseDown(MouseButton mouseButton = MOUSE_BUTTON_LEFT) const = 0;
            virtual bool IsMouseUp(MouseButton mouseButton = MOUSE_BUTTON_LEFT) const = 0;
            virtual bool IsKeyDown(Key key) const = 0;
            virtual bool IsKeyUp(Key key) const = 0;
        protected:
            CmWindow() = default;
    };
}
