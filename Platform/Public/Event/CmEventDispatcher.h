#pragma once

#include "CmWindowEvent.h"
#include "CmMouseEvent.h"

namespace chimi
{
    class CmEventObserver;
    using Func_EventHandle = std::function<void(CmEvent& e)>;

    struct EventHandler{
        CmEventObserver *observer;
        Func_EventHandle func;
    };

    class CmEventDispatcher {
    public:
        CmEventDispatcher(const CmEventDispatcher&) = delete;
        CmEventDispatcher &operator=(const CmEventDispatcher&) = delete;
        ~CmEventDispatcher();

        static CmEventDispatcher* GetInstance() { return &s_Instance; }

        template<typename T>
        void CmdObserverHandler(CmEventObserver *observer, const std::function<void(const T &)> &func){
            if(!observer || !func){
                return;
            }

            auto eventFunc = [func](const CmEvent &e){
                const T &event = static_cast<const T&>(e);
                return func(event);
            };

            EventHandler handler {
                .observer = observer,
                .func = eventFunc
            };
            mObserverHandlerMap[T::GetStaticType()].emplace_back(handler);
        }

        void DestroyObserver(CmEventObserver *observer){
            for (auto & mapIt : mObserverHandlerMap) {
                mapIt.second.erase(std::remove_if(mapIt.second.begin(), mapIt.second.end(), [observer](const EventHandler& handler) {
                    return (handler.observer && handler.observer == observer);
                }), mapIt.second.end());
            }
        }

        void Dispatch(CmEvent &event);
    private:
        CmEventDispatcher() = default;

        static CmEventDispatcher s_Instance;

        std::unordered_map<CmEventType, std::vector<EventHandler>> mObserverHandlerMap;
    };
}
