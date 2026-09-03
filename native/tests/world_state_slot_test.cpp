#include "engine/WorldStateSlotMap.h"

#include <cassert>
#include <cstdio>

using ico::engine::FindWorldStateSlotBinding;
using ico::engine::WorldStateSlotBinding;

int main()
{
    // === shareGroup 2: slot 0x1A / BSS 0x006782F8 (dominant, 2.07M events) ===
    const WorldStateSlotBinding* b = FindWorldStateSlotBinding(0x0F);
    assert(b != nullptr);
    assert(b->slotIndex == 0x1A);
    assert(b->bssAddress == 0x006782F8);
    assert(b->shareGroup == 2);

    const WorldStateSlotBinding* b9 = FindWorldStateSlotBinding(0x09);
    assert(b9 != nullptr && b9->slotIndex == 0x1A);
    assert(b9->shareGroup == b->shareGroup);

    const WorldStateSlotBinding* be = FindWorldStateSlotBinding(0x0E);
    assert(be != nullptr && be->slotIndex == 0x1A);
    assert(be->shareGroup == b->shareGroup);

    // Rev.115: 0x19 also uses slot 0x1A (98.6%)
    const WorldStateSlotBinding* b19 = FindWorldStateSlotBinding(0x19);
    assert(b19 != nullptr && b19->slotIndex == 0x1A);
    assert(b19->shareGroup == b->shareGroup);

    // === shareGroup 8: slot 0x21 / BSS 0x006794E8 ===
    const WorldStateSlotBinding* a = FindWorldStateSlotBinding(0x0A);
    assert(a != nullptr && a->slotIndex == 0x21);
    assert(a->bssAddress == 0x006794E8);

    const WorldStateSlotBinding* a3 = FindWorldStateSlotBinding(0x03);
    assert(a3 != nullptr && a3->slotIndex == 0x21);
    assert(a3->shareGroup == a->shareGroup);

    const WorldStateSlotBinding* a2b = FindWorldStateSlotBinding(0x2B);
    assert(a2b != nullptr && a2b->slotIndex == 0x21);
    assert(a2b->shareGroup == a->shareGroup);

    // Rev.115: 0x29 also uses slot 0x21 (weak, 58.3%)
    const WorldStateSlotBinding* a29 = FindWorldStateSlotBinding(0x29);
    assert(a29 != nullptr && a29->slotIndex == 0x21);

    // === Distinct slot distribution for different rooms ===
    assert(FindWorldStateSlotBinding(0x0D)->slotIndex == 0x1C);
    assert(FindWorldStateSlotBinding(0x06)->slotIndex == 0x1E);
    assert(FindWorldStateSlotBinding(0x07)->slotIndex == 0x1F);
    assert(FindWorldStateSlotBinding(0x08)->slotIndex == 0x20);
    assert(FindWorldStateSlotBinding(0x01)->slotIndex == 0x22);
    assert(FindWorldStateSlotBinding(0x04)->slotIndex == 0x33);
    assert(FindWorldStateSlotBinding(0x2D)->slotIndex == 0x40);
    assert(FindWorldStateSlotBinding(0x28)->slotIndex == 0x29);
    assert(FindWorldStateSlotBinding(0x0B)->slotIndex == 0x44);
    assert(FindWorldStateSlotBinding(0x05)->slotIndex == 0x18);

    // === Rev.115: new world_states from 2.8M event session ===
    assert(FindWorldStateSlotBinding(0x1A)->slotIndex == 0x1D);  /* 99.9% */
    assert(FindWorldStateSlotBinding(0x1B)->slotIndex == 0x26);  /* 99.8% */
    assert(FindWorldStateSlotBinding(0x1C)->slotIndex == 0x2E);  /* 98.3% */
    assert(FindWorldStateSlotBinding(0x1D)->slotIndex == 0x36);  /* 99.1% */
    assert(FindWorldStateSlotBinding(0x16)->slotIndex == 0x1E);  /* 99.7% */
    assert(FindWorldStateSlotBinding(0x11)->slotIndex == 0x1D);  /* 98.8% */
    assert(FindWorldStateSlotBinding(0x14)->slotIndex == 0x36);  /* 96.9% */
    assert(FindWorldStateSlotBinding(0x13)->slotIndex == 0x34);  /* 98.7% */
    assert(FindWorldStateSlotBinding(0x10)->slotIndex == 0x1F);  /* 98.9% */
    assert(FindWorldStateSlotBinding(0x17)->slotIndex == 0x23);  /* 98.7% */
    assert(FindWorldStateSlotBinding(0x15)->slotIndex == 0x26);  /* 92.9% */
    assert(FindWorldStateSlotBinding(0x12)->slotIndex == 0x25);  /* 98.2% */
    assert(FindWorldStateSlotBinding(0x18)->slotIndex == 0x1C);  /* 96.1% */
    assert(FindWorldStateSlotBinding(0x1E)->slotIndex == 0x24);  /* 93.4% */
    assert(FindWorldStateSlotBinding(0x33)->slotIndex == 0x18);  /* 93.7% */
    assert(FindWorldStateSlotBinding(0x32)->slotIndex == 0x18);  /* 93.9% */

    // === Zone sharing verification ===
    // 0x1A and 0x11 share slot 0x1D (shareGroup 4)
    assert(FindWorldStateSlotBinding(0x1A)->shareGroup ==
           FindWorldStateSlotBinding(0x11)->shareGroup);
    // 0x16 and 0x06 share slot 0x1E (shareGroup 5)
    assert(FindWorldStateSlotBinding(0x16)->shareGroup ==
           FindWorldStateSlotBinding(0x06)->shareGroup);
    // 0x10 and 0x07 share slot 0x1F (shareGroup 6)
    assert(FindWorldStateSlotBinding(0x10)->shareGroup ==
           FindWorldStateSlotBinding(0x07)->shareGroup);
    // 0x1B and 0x15 share slot 0x26 (shareGroup 13)
    assert(FindWorldStateSlotBinding(0x1B)->shareGroup ==
           FindWorldStateSlotBinding(0x15)->shareGroup);
    // 0x1D and 0x14 share slot 0x36 (shareGroup 18)
    assert(FindWorldStateSlotBinding(0x1D)->shareGroup ==
           FindWorldStateSlotBinding(0x14)->shareGroup);
    // 0x05, 0x33, 0x32 share slot 0x18 (shareGroup 1)
    assert(FindWorldStateSlotBinding(0x05)->shareGroup ==
           FindWorldStateSlotBinding(0x33)->shareGroup);
    assert(FindWorldStateSlotBinding(0x05)->shareGroup ==
           FindWorldStateSlotBinding(0x32)->shareGroup);

    // === World states without a confirmed dominant slot return nullptr ===
    assert(FindWorldStateSlotBinding(0x00) == nullptr);
    assert(FindWorldStateSlotBinding(0x02) == nullptr);
    assert(FindWorldStateSlotBinding(0x0C) == nullptr);

    // Verify total binding count
    assert(ico::engine::kWorldStateSlotBindingCount == 35);

    std::printf("world_state_slot_test: all passed (36 bindings, 20 slots, 20 share groups)\n");
    return 0;
}
