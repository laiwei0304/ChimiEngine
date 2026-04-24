#pragma once

#include "CmEventDispatcher.h"

namespace chimi
{
    class CmEventObserver{
    public:
        CmEventObserver() = default;
        ~CmEventObserver(){
            CmEventDispatcher::GetInstance()->DestroyObserver(this);
        }

        template<typename T>
        void OnEvent(const std::function<void(const T &)> &func){
            CmEventDispatcher::GetInstance()->CmdObserverHandler(this, func);
        }
    };
}
