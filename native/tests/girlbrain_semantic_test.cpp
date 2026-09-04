#include "game/GirlBrainSemantic.h"

#include <cassert>

namespace {
ico_ptr32 g_observedAddress = 0;

void recordTargetState(ico_ptr32 targetStateAddress) {
    g_observedAddress = targetStateAddress;
}
}  // namespace

int main() {
    assert(!girlBrainClearTarget(nullptr));
    assert(girlBrainClearTarget(recordTargetState));
    assert(g_observedAddress == kGirlBrainTargetStateAddress);
    return 0;
}
