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

            static std::unique_ptr<CmWindow> Create(uint32_t width, uint32_t height, const char *title);

            virtual bool ShouldClose() = 0;
            virtual void PollEvents() = 0;
            virtual void SwapBuffer() = 0;

            virtual void* GetImplWindowPointer() const = 0;

            virtual void GetMousePos(glm::vec2 &mousePos) const = 0;
            virtual bool IsMouseDown(MouseButton mouseButton = MOUSE_BUTTON_LEFT) const = 0;
            virtual bool IsMouseUp(MouseButton mouseButton = MOUSE_BUTTON_LEFT) const = 0;
            virtual bool IsKeyDown(Key key) const = 0;
            virtual bool IsKeyUp(Key key) const = 0;
        protected:
            CmWindow() = default;
    };
}
