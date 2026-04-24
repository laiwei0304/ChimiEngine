#pragma once

#include "Event/CmEventObserver.h"

class CmEventTesting{
public:
    CmEventTesting();
    ~CmEventTesting();
private:
    void TestMemberFunc(const chimi::CmMouseButtonReleaseEvent &event);

    std::shared_ptr<chimi::CmEventObserver> mObserver;
};
