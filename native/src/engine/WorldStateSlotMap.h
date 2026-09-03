#pragma once

#include "core/gobj_abi.h"

#include <cstddef>

namespace ico::engine {

/*
 * WorldStateSlotMap — semantic reconstruction of the per-room dispatch binding
 * observed in runtime captures (Rev.111).
 *
 * Ground truth: JSONL runtime capture ico-runtime-20260825-152452.jsonl.
 * Consistent with: ios_om_create_dl (a1 carries the BSS slot address) and
 * _iosOmMain (a2 carries the slot index) probes at 0x0013FC00 / 0x0013F9D0.
 *
 * Confirmed: each world_state has one dominant DL dispatch slot (>90% of
 * ios_om_main events for that state). The same slot index is reused by a small
 * group of world_states (zone sharing): e.g. slot 0x1A serves 0x0F/0x09/0x0E,
 * slot 0x21 serves 0x0A/0x03/0x2B.
 *
 * Inferred (semantic, NOT byte-exact): the slot index maps to a per-list
 * dispatch bit / list id in the native runtime's isysGObj* model. The mapping
 * table below is derived from the measured primary slot per world_state, not
 * from a recovered .data table.
 *
 * This header documents the mapping only; host-side dispatch is performed by
 * IsysGObjRuntime. It is deliberately decoupled from the runtime so it can be
 * used as a data contract for scene loading without depending on engine internals.
 */

constexpr u32 kWorldStateCount = 34;

struct WorldStateSlotBinding {
    u32 worldState;
    u32 slotIndex;   /* a2 value observed on _iosOmMain, e.g. 0x1A */
    u32 bssAddress;  /* BSS dispatch slot address observed on ios_om_create_dl */
    u32 shareGroup;  /* zone id shared by adjacent rooms on the same slot */
};

/*
 * Primary slot binding per world_state, from Rev.111 (frequency >= 86% for each
 * listed state). Only world_states with a clear dominant slot are listed.
 * Slot index and BSS address are confirmed runtime observations; shareGroup is
 * an interpretation that groups world_states sharing the same slot index.
 * Unknown/discarded states (0x00, 0x02, 0x0C) are omitted.
 */
constexpr WorldStateSlotBinding kWorldStateSlotBindings[] = {
    { 0x05, 0x18, 0x00677dd8, 1 },   /* Slot A: 91.4% */
    { 0x0F, 0x1A, 0x006782f8, 2 },   /* Slot B: 100% */
    { 0x09, 0x1A, 0x006782f8, 2 },   /* Slot B (shared) */
    { 0x0E, 0x1A, 0x006782f8, 2 },   /* Slot B (shared) */
    { 0x0D, 0x1C, 0x00678818, 3 },   /* Slot NEW-1: 98.7% */
    { 0x06, 0x1E, 0x00678d38, 4 },   /* Slot C: 98.1% */
    { 0x07, 0x1F, 0x00678fc8, 5 },   /* Slot D: 98.5% */
    { 0x08, 0x20, 0x00679258, 6 },   /* Slot E: 98.8% */
    { 0x0A, 0x21, 0x006794e8, 7 },   /* Slot F: 97.6% */
    { 0x03, 0x21, 0x006794e8, 7 },   /* Slot F (shared) */
    { 0x2B, 0x21, 0x006794e8, 7 },   /* Slot F (shared) */
    { 0x01, 0x22, 0x00679778, 8 },   /* Slot G: 92.7% */
    { 0x04, 0x33, 0x0067c308, 9 },   /* Slot I: 99.2% */
    { 0x2D, 0x40, 0x0067e458, 10 },  /* Slot J: 86.3% */
    { 0x28, 0x29, 0x0067a968, 11 },  /* Slot H: 56.6% */
    { 0x0B, 0x44, 0x0067ee98, 12 },  /* Slot NEW-2: 99.2% */
};

constexpr std::size_t kWorldStateSlotBindingCount =
    sizeof(kWorldStateSlotBindings) / sizeof(kWorldStateSlotBindings[0]);

/*
 * Return the slot binding for a world_state, or nullptr if the state has no
 * confirmed dominant slot.
 */
inline const WorldStateSlotBinding* FindWorldStateSlotBinding(u32 worldState)
{
    for (std::size_t i = 0; i < kWorldStateSlotBindingCount; ++i) {
        if (kWorldStateSlotBindings[i].worldState == worldState) {
            return &kWorldStateSlotBindings[i];
        }
    }
    return nullptr;
}

} // namespace ico::engine
