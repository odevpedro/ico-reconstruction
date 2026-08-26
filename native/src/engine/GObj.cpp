#include "engine/GObj.h"

#include <cstring>

namespace ico::engine {

void resetGObjSlot(GObj& gobj)
{
    std::memset(&gobj, 0, sizeof(gobj));
    gobj.unknown_004 = -1;
    gobj.unknown_008 = -1;
}

bool isGObjSlotFree(const GObj& gobj)
{
    return gobj.self == kNullGObjHandle;
}

} // namespace ico::engine
