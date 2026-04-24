#pragma once

#include "CmEntity.h"

namespace chimi{
    class CmComponent{
    public:
        void SetOwner(CmEntity *owner){ mOwner = owner; }
        CmEntity *GetOwner() const { return mOwner; }
    private:
        CmEntity *mOwner = nullptr;
    };
}
