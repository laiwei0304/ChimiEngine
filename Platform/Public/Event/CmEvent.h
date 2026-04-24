#pragma once

#include "CmEngine.h"

namespace chimi
{
    enum CmEventType{
        // Window
        EVENT_TYPE_FRAME_BUFFER_RESIZE,                  /*      window framebuffer resize */
        EVENT_TYPE_WINDOW_FOCUS,                         /*      window focus              */
        EVENT_TYPE_WINDOW_LOST_FOCUS,                    /*      window lost focus         */
        EVENT_TYPE_WINDOW_MOVED,                         /*      window moved              */
        EVENT_TYPE_WINDOW_CLOSE,                         /*      window close              */
        // Key
        EVENT_TYPE_KEY_PRESS,                            /*      press key                 */
        EVENT_TYPE_KEY_RELEASE,                          /*      release key               */
        // Mouse
        EVENT_TYPE_MOUSE_PRESS,                          /*      press mouse button        */
        EVENT_TYPE_MOUSE_RELEASE,                        /*      release mouse button      */
        EVENT_TYPE_MOUSE_MOVED,                          /*      move mouse position       */
        EVENT_TYPE_MOUSE_SCROLLED,                       /*      mouse scroll              */
    };

#define EVENT_CLASS_TYPE(type) static chimi::CmEventType GetStaticType(){ return chimi::CmEventType::type; }         \
virtual chimi::CmEventType GetEventType() const override{ return GetStaticType(); } \
virtual const char* GetName() const override{ return #type; }                     \

    class CmEvent{
    public:
        virtual ~CmEvent() = default;

        [[nodiscard]] virtual CmEventType GetEventType() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual std::string ToString() const { return GetName(); };
    };
}
