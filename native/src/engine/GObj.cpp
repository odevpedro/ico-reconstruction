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

void resetProcessNodeSlot(ProcessNode& process)
{
    std::memset(&process, 0, sizeof(process));
}

bool isProcessNodeSlotFree(const ProcessNode& process)
{
    return process.self == kNullProcessHandle;
}

} // namespace ico::engine
