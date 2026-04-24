#pragma once

#include "CmEngine.h"

namespace chimi{
    class CmUUID{
    public:
        CmUUID();
        CmUUID(uint32_t uuid);
        CmUUID(const CmUUID&) = default;
        operator uint32_t() const { return mUUID; }

        uint64_t mUUID;
    };
}

namespace std {
    template<>
    struct hash<chimi::CmUUID>{
        std::size_t operator()(const chimi::CmUUID& uuid) const{
            if(!uuid){
                return 0;
            }
            return (uint32_t)uuid;
        }
    };
}
