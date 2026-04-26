#include "ECS/CmUUID.h"
#include <random>

namespace chimi{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<uint32_t> s_UniformDistribution(1, UINT32_MAX);

    CmUUID::CmUUID() : mUUID(s_UniformDistribution(s_Engine)) {

    }

    CmUUID::CmUUID(uint32_t uuid) : mUUID(uuid) {

    }
}