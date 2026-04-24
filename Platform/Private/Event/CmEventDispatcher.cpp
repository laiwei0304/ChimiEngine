#include "Event/CmEventDispatcher.h"
#include "spdlog/stopwatch.h"
#include "CmLog.h"

namespace chimi{
    CmEventDispatcher CmEventDispatcher::s_Instance{};

    CmEventDispatcher::~CmEventDispatcher() {
        mObserverHandlerMap.clear();
    }

    void CmEventDispatcher::Dispatch(CmEvent &event) {
        if(mObserverHandlerMap.find(event.GetEventType()) == mObserverHandlerMap.end()){
            return;
        }
        auto observers = mObserverHandlerMap[event.GetEventType()];
        if(observers.empty()){
            return;
        }

        spdlog::stopwatch stopwatch;
        stopwatch.reset();
        for (const auto &observer: observers){
            if(observer.observer){
                observer.func(event);
            }
        }
        LOG_T("observer count: {0}, timing: {1:.2}ms", observers.size(), stopwatch.elapsed().count() * 1000);
    }
}