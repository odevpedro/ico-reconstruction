#include "engine/WorldStateSlotMap.h"

#include <cassert>

using ico::engine::FindWorldStateSlotBinding;
using ico::engine::WorldStateSlotBinding;

int main()
{
    // Confirmed dominant slots from Rev.111 runtime capture.
    // Slot B (0x1A) is shared by 0x0F, 0x09, 0x0E -> same share group.
    const WorldStateSlotBinding* b = FindWorldStateSlotBinding(0x0F);
    assert(b != nullptr);
    assert(b->slotIndex == 0x1A);
    assert(b->bssAddress == 0x006782f8);
    assert(b->shareGroup == 2);

    const WorldStateSlotBinding* b9 = FindWorldStateSlotBinding(0x09);
    assert(b9 != nullptr && b9->slotIndex == 0x1A);
    assert(b9->shareGroup == b->shareGroup);

    const WorldStateSlotBinding* be = FindWorldStateSlotBinding(0x0E);
    assert(be != nullptr && be->slotIndex == 0x1A);
    assert(be->shareGroup == b->shareGroup);

    // Slot F (0x21) is shared by 0x0A, 0x03, 0x2B -> same share group.
    const WorldStateSlotBinding* a = FindWorldStateSlotBinding(0x0A);
    assert(a != nullptr && a->slotIndex == 0x21);
    assert(a->bssAddress == 0x006794e8);

    const WorldStateSlotBinding* a3 = FindWorldStateSlotBinding(0x03);
    assert(a3 != nullptr && a3->slotIndex == 0x21);
    assert(a3->shareGroup == a->shareGroup);

    const WorldStateSlotBinding* a2b = FindWorldStateSlotBinding(0x2B);
    assert(a2b != nullptr && a2b->slotIndex == 0x21);
    assert(a2b->shareGroup == a->shareGroup);

    // Slot distribution distinct for different rooms.
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

    // World states without a confirmed dominant slot return nullptr.
    // 0x00, 0x02, 0x0C are not mapped in Rev.111.
    assert(FindWorldStateSlotBinding(0x00) == nullptr);
    assert(FindWorldStateSlotBinding(0x02) == nullptr);
    assert(FindWorldStateSlotBinding(0x0C) == nullptr);

    return 0;
}
