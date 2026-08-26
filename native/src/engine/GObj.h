#pragma once

#include "core/gobj_abi.h"

namespace ico::engine {

using GObj = IcoGObj;
using ProcessNode = IcoProcessNode;
using GObjHandle = ico_ptr32;

constexpr GObjHandle kNullGObjHandle = 0;
constexpr u32 kPrimaryListCount = ICO_GOBJ_PRIMARY_LIST_COUNT;
constexpr u32 kDlListCount = ICO_GOBJ_DL_LIST_COUNT;
constexpr u32 kDlMaskBits = ICO_GOBJ_DL_MASK_BITS;

void resetGObjSlot(GObj& gobj);
bool isGObjSlotFree(const GObj& gobj);

static_assert(sizeof(GObj) == 0x174, "native GObj must preserve the ICO ABI");
static_assert(sizeof(ProcessNode) == 0x94,
              "native ProcessNode must preserve the ICO ABI");

} // namespace ico::engine
