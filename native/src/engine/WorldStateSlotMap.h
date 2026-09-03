#pragma once

#include "core/gobj_abi.h"

#include <cstddef>

namespace ico::engine {

/*
 * WorldStateSlotMap — semantic reconstruction of the per-room dispatch binding
 * observed in runtime captures (Rev.115).
 *
 * Ground truth: JSONL runtime capture ico-runtime-20260825-152452.jsonl
 * (2,807,253 events, 2,070,225 ios_om_main hits, 129,925 ios_om_create_dl hits).
 *
 * Confirmed via correlated ios_om_main (a2=slot_index) and ios_om_create_dl
 * (a1=BSS_address) probes at 0x0013F9D0 and 0x0013FC00.
 *
 * 20 unique BSS dispatch slot addresses observed in BSS range 0x00677DD8-0x0067EE98.
 * Each world_state has one dominant slot (>87% of ios_om_main events for that state).
 * Zone sharing: multiple world_states share the same slot index (e.g. 0x0F/0x09/0x0E
 * all use slot 0x1A / BSS 0x006782F8).
 *
 * Slot index spacing is approximately 0x290 bytes between consecutive BSS addresses,
 * consistent with a fixed-size per-slot data structure.
 *
 * This header documents the mapping only; host-side dispatch is performed by
 * IsysGObjRuntime. It is deliberately decoupled from the runtime so it can be
 * used as a data contract for scene loading without depending on engine internals.
 */

constexpr u32 kWorldStateCount = 35;

struct WorldStateSlotBinding {
    u32 worldState;
    u32 slotIndex;   /* a2 value observed on _iosOmMain */
    u32 bssAddress;  /* BSS dispatch slot address observed on ios_om_create_dl */
    u32 shareGroup;  /* zone id: groups world_states sharing the same slot */
};

/*
 * Primary slot binding per world_state (frequency >= 87% for each listed state).
 * Slot index and BSS address are confirmed runtime observations; shareGroup
 * groups world_states sharing the same slot index.
 *
 * Slot BSS addresses (all confirmed):
 *   0x18=0x00677DD8  0x1A=0x006782F8  0x1C=0x00678818  0x1D=0x00678AA8
 *   0x1E=0x00678D38  0x1F=0x00678FC8  0x20=0x00679258  0x21=0x006794E8
 *   0x22=0x00679778  0x23=0x00679A08  0x24=0x00679C98  0x25=0x00679F28
 *   0x26=0x0067A1B8  0x29=0x0067A968  0x2E=0x0067B638  0x33=0x0067C308
 *   0x34=0x0067C598  0x36=0x0067CAB8  0x40=0x0067E458  0x44=0x0067EE98
 */
constexpr WorldStateSlotBinding kWorldStateSlotBindings[] = {
    /*  shareGroup 1: slot 0x18 / BSS 0x00677DD8 */
    { 0x05, 0x18, 0x00677DD8, 1 },   /* 94.6% */
    { 0x33, 0x18, 0x00677DD8, 1 },   /* 93.7% */
    { 0x32, 0x18, 0x00677DD8, 1 },   /* 93.9% */

    /*  shareGroup 2: slot 0x1A / BSS 0x006782F8 — dominant (2.07M events) */
    { 0x0F, 0x1A, 0x006782F8, 2 },   /* 100.0% */
    { 0x09, 0x1A, 0x006782F8, 2 },   /* 97.9% */
    { 0x0E, 0x1A, 0x006782F8, 2 },   /* 98.1% */
    { 0x19, 0x1A, 0x006782F8, 2 },   /* 98.6% */

    /*  shareGroup 3: slot 0x1C / BSS 0x00678818 */
    { 0x0D, 0x1C, 0x00678818, 3 },   /* 98.8% */
    { 0x18, 0x1C, 0x00678818, 3 },   /* 96.1% */

    /*  shareGroup 4: slot 0x1D / BSS 0x00678AA8 — second most active (29K create_dl) */
    { 0x1A, 0x1D, 0x00678AA8, 4 },   /* 99.9% */
    { 0x11, 0x1D, 0x00678AA8, 4 },   /* 98.8% */

    /*  shareGroup 5: slot 0x1E / BSS 0x00678D38 */
    { 0x16, 0x1E, 0x00678D38, 5 },   /* 99.7% */
    { 0x06, 0x1E, 0x00678D38, 5 },   /* 98.2% */

    /*  shareGroup 6: slot 0x1F / BSS 0x00678FC8 */
    { 0x10, 0x1F, 0x00678FC8, 6 },   /* 98.9% */
    { 0x07, 0x1F, 0x00678FC8, 6 },   /* 98.8% */

    /*  shareGroup 7: slot 0x20 / BSS 0x00679258 */
    { 0x08, 0x20, 0x00679258, 7 },   /* 98.6% */

    /*  shareGroup 8: slot 0x21 / BSS 0x006794E8 */
    { 0x0A, 0x21, 0x006794E8, 8 },   /* 98.9% */
    { 0x03, 0x21, 0x006794E8, 8 },   /* 93.0% */
    { 0x2B, 0x21, 0x006794E8, 8 },   /* 92.4% */
    { 0x29, 0x21, 0x006794E8, 8 },   /* 58.3% (weak) */

    /*  shareGroup 9: slot 0x22 / BSS 0x00679778 */
    { 0x01, 0x22, 0x00679778, 9 },   /* 90.7% */
    { 0x2A, 0x22, 0x00679778, 9 },   /* 60.2% (weak) */

    /*  shareGroup 10: slot 0x23 / BSS 0x00679A08 */
    { 0x17, 0x23, 0x00679A08, 10 },  /* 98.7% */

    /*  shareGroup 11: slot 0x24 / BSS 0x00679C98 */
    { 0x1E, 0x24, 0x00679C98, 11 },  /* 93.4% */

    /*  shareGroup 12: slot 0x25 / BSS 0x00679F28 */
    { 0x12, 0x25, 0x00679F28, 12 },  /* 98.2% */

    /*  shareGroup 13: slot 0x26 / BSS 0x0067A1B8 — high activity (15K create_dl) */
    { 0x1B, 0x26, 0x0067A1B8, 13 },  /* 99.8% */
    { 0x15, 0x26, 0x0067A1B8, 13 },  /* 92.9% */

    /*  shareGroup 14: slot 0x29 / BSS 0x0067A968 */
    { 0x28, 0x29, 0x0067A968, 14 },  /* 67.9% (weak) */

    /*  shareGroup 15: slot 0x2E / BSS 0x0067B638 */
    { 0x1C, 0x2E, 0x0067B638, 15 },  /* 98.3% */

    /*  shareGroup 16: slot 0x33 / BSS 0x0067C308 */
    { 0x04, 0x33, 0x0067C308, 16 },  /* 98.9% */

    /*  shareGroup 17: slot 0x34 / BSS 0x0067C598 */
    { 0x13, 0x34, 0x0067C598, 17 },  /* 98.7% */

    /*  shareGroup 18: slot 0x36 / BSS 0x0067CAB8 — high activity (15K create_dl) */
    { 0x1D, 0x36, 0x0067CAB8, 18 },  /* 99.1% */
    { 0x14, 0x36, 0x0067CAB8, 18 },  /* 96.9% */

    /*  shareGroup 19: slot 0x40 / BSS 0x0067E458 */
    { 0x2D, 0x40, 0x0067E458, 19 },  /* 87.3% */

    /*  shareGroup 20: slot 0x44 / BSS 0x0067EE98 */
    { 0x0B, 0x44, 0x0067EE98, 20 },  /* 99.2% */
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
