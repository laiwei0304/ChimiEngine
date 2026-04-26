#pragma once

#include "Event/CmKeyEvent.h"

namespace chimi
{
    enum MouseButton{
        MOUSE_BUTTON_LEFT   =   0,
        MOUSE_BUTTON_RIGHT  =   1,
        MOUSE_BUTTON_MIDDLE =   2,
    };

    static const char* CmMouseButtonToStr(MouseButton mouseButton){
        switch (mouseButton) {
            ENUM_TO_STR(MOUSE_BUTTON_LEFT);
            ENUM_TO_STR(MOUSE_BUTTON_RIGHT);
            ENUM_TO_STR(MOUSE_BUTTON_MIDDLE);
        }
        return "unknown";
    }

    class CmMouseButtonPressEvent : public CmEvent{
        public:
            CmMouseButtonPressEvent(MouseButton mouseButton, KeyMod keyMod, bool repeat) : mMouseButton(mouseButton), mKeyMod(keyMod), mRepeat(repeat) {
            };

            [[nodiscard]] std::string ToString() const override {
                std::stringstream ss;
                ss << CmEvent::ToString();
                ss << "( mouseButton=" << CmMouseButtonToStr(mMouseButton);
                ss << ", keyMod=" << CmKeyModToStr(mKeyMod);
                ss << ", isRepeat=" << mRepeat << " )";
                return ss.str();
            }

            [[nodiscard]] bool IsShiftPressed() const { return mKeyMod & CHIMI_MOD_SHIFT; };
            [[nodiscard]] bool IsControlPressed() const { return mKeyMod & CHIMI_MOD_CONTROL; };
            [[nodiscard]] bool IsAltPressed() const { return mKeyMod & CHIMI_MOD_ALT; };
            [[nodiscard]] bool IsSuperPressed() const { return mKeyMod & CHIMI_MOD_SUPER; };
            [[nodiscard]] bool IsCapsLockPressed() const { return mKeyMod & CHIMI_MOD_CAPS_LOCK; };    //TODO mod do not return this.
            [[nodiscard]] bool IsNumLockPressed() const { return mKeyMod & CHIMI_MOD_NUM_LOCK; };      //TODO mod do not return this.
            [[nodiscard]] bool IsRepeat() const { return mRepeat; };

            MouseButton mMouseButton;
            KeyMod mKeyMod;
            bool mRepeat;
            EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_PRESS);
    };

    class CmMouseButtonReleaseEvent : public CmEvent{
        public:
            CmMouseButtonReleaseEvent(MouseButton mouseButton) : mMouseButton(mouseButton){

            };
            [[nodiscard]] std::string ToString() const override {
                std::stringstream ss;
                ss << CmEvent::ToString();
                ss << "( mouseButton=" << CmMouseButtonToStr(mMouseButton) << " )";
                return ss.str();
            }

            MouseButton mMouseButton;
            EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_RELEASE);
    };

    class CmMouseMovedEvent : public CmEvent{
        public:
            CmMouseMovedEvent(float xPos, float yPos) : mXPos(xPos), mYPos(yPos) {

            }
            [[nodiscard]] std::string ToString() const override {
                std::stringstream ss;
                ss << CmEvent::ToString();
                ss << "( xPos=" << mXPos;
                ss << ", yPos=" << mYPos << " )";
                return ss.str();
            }

            float mXPos;
            float mYPos;
            EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_MOVED);
    };

    class CmMouseScrollEvent : public CmEvent{
        public:
            CmMouseScrollEvent(float xOffset, float yOffset) : mXOffset(xOffset), mYOffset(yOffset) {};
            [[nodiscard]] std::string ToString() const override {
                std::stringstream ss;
                ss << CmEvent::ToString();
                ss << "( xOffset=" << mXOffset;
                ss << ", yOffset=" << mYOffset << " )";
                return ss.str();
            }

            float mXOffset;
            float mYOffset;
            EVENT_CLASS_TYPE(EVENT_TYPE_MOUSE_SCROLLED);
    };
}
