#include "CmEventTesting.h"
#include "CmLog.h"

#define BIND_EVENT_FN(fn)           [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define REG_EVENT(type) mObserver->OnEvent<type>([](const type &event){ LOG_T("{0}", event.ToString()); });

static void TestStaticFunc(const chimi::CmMouseButtonPressEvent &event){
    LOG_T("{0}", event.ToString());
}

void CmEventTesting::TestMemberFunc(const chimi::CmMouseButtonReleaseEvent &event) {
    LOG_T("{0}", event.ToString());
}

CmEventTesting::CmEventTesting() {
    mObserver = std::make_shared<chimi::CmEventObserver>();
    mObserver->OnEvent<chimi::CmFrameBufferResizeEvent>([](const chimi::CmFrameBufferResizeEvent &event){ LOG_T("{0}", event.ToString()); });
    mObserver->OnEvent<chimi::CmMouseButtonPressEvent>(TestStaticFunc);
    mObserver->OnEvent<chimi::CmMouseButtonReleaseEvent>(BIND_EVENT_FN(TestMemberFunc));
    REG_EVENT(chimi::CmWindowFocusEvent);
    REG_EVENT(chimi::CmWindowLostFocusEvent);
    REG_EVENT(chimi::CmWindowMovedEvent);
    REG_EVENT(chimi::CmWindowCloseEvent);
    REG_EVENT(chimi::CmKeyPressEvent);
    REG_EVENT(chimi::CmKeyReleaseEvent);
//    REG_EVENT(chimi::CmMouseMovedEvent);
    REG_EVENT(chimi::CmMouseScrollEvent);
}

CmEventTesting::~CmEventTesting() {
    mObserver.reset();
}
