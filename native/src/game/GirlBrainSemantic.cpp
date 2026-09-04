#include "game/GirlBrainSemantic.h"

bool girlBrainClearTarget(GirlBrainTargetClearFn clearTarget) {
    if (clearTarget == nullptr) {
        return false;
    }
    clearTarget(kGirlBrainTargetStateAddress);
    return true;
}
