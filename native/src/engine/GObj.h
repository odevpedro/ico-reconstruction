#pragma once

#include "core/gobj_abi.h"

namespace ico::engine {

using GObj = IcoGObj;
using ProcessNode = IcoProcessNode;
using GObjHandle = ico_ptr32;
using ProcessHandle = ico_ptr32;

constexpr GObjHandle kNullGObjHandle = 0;
constexpr ProcessHandle kNullProcessHandle = 0;
constexpr u32 kPrimaryListCount = ICO_GOBJ_PRIMARY_LIST_COUNT;
constexpr u32 kDlListCount = ICO_GOBJ_DL_LIST_COUNT;
constexpr u32 kDlMaskBits = ICO_GOBJ_DL_MASK_BITS;
constexpr u32 kTypeTableEntries = ICO_GOBJ_TYPE_TABLE_ENTRIES;
constexpr u32 kTypeSlotStart = 0x13;
constexpr u32 kTypeSlotCount = 9;
constexpr u32 kTypeSlotEnd = kTypeSlotStart + kTypeSlotCount;

void resetGObjSlot(GObj& gobj);
bool isGObjSlotFree(const GObj& gobj);
void resetProcessNodeSlot(ProcessNode& process);
bool isProcessNodeSlotFree(const ProcessNode& process);

static_assert(sizeof(GObj) == 0x174, "native GObj must preserve the ICO ABI");
static_assert(sizeof(ProcessNode) == 0x94,
              "native ProcessNode must preserve the ICO ABI");

} // namespace ico::engine
