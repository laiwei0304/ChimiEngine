#pragma once

#include "Event/CmEvent.h"

namespace chimi
{
    class CmFrameBufferResizeEvent : public CmEvent{
    public:
        CmFrameBufferResizeEvent(uint32_t width, uint32_t height) : mWidth(width), mHeight(height) {}
        [[nodiscard]] std::string ToString() const override {
            return CmEvent::ToString() + "( with=" + std::to_string(mWidth) + ", height=" + std::to_string(mHeight) + " )";
        }

        uint32_t mWidth, mHeight;
        EVENT_CLASS_TYPE(EVENT_TYPE_FRAME_BUFFER_RESIZE);
    };

    class CmWindowFocusEvent : public CmEvent{
    public:
        EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_FOCUS);
    };

    class CmWindowLostFocusEvent : public CmEvent{
    public:
        EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_LOST_FOCUS);
    };

    class CmWindowMovedEvent : public CmEvent{
    public:
        CmWindowMovedEvent(uint32_t xPos, uint32_t yPos) : mXPos(xPos), mYPos(yPos) {};
        [[nodiscard]] std::string ToString() const override {
            return CmEvent::ToString() + "( xPos=" + std::to_string(mXPos) + ", yPos=" + std::to_string(mYPos) + " )";
        }

        uint32_t mXPos;
        uint32_t mYPos;
        EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_MOVED);
    };

    class CmWindowCloseEvent : public CmEvent{
    public:
        EVENT_CLASS_TYPE(EVENT_TYPE_WINDOW_CLOSE);
    };
}
