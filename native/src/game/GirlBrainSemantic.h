#pragma once

#include "ps2/Ps2Types.h"

/*
 * Native semantic boundary for GirlBrainClearTarget (USA 0x0016AC10).
 * The original 16-byte function tail-calls 0x00182890 with a0=0x0028A890.
 * The callee's source-level contract has not been recovered; callers supply
 * its portable implementation explicitly.
 */
using GirlBrainTargetClearFn = void (*)(ico_ptr32 targetStateAddress);

constexpr ico_ptr32 kGirlBrainTargetStateAddress = 0x0028A890;

bool girlBrainClearTarget(GirlBrainTargetClearFn clearTarget);
