#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern const char* hosi_name[];

s32 get_hosi_name(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 entryId = evtGetValue(event, args[0]);
    evtSetValue(event, args[1], (s32)hosi_name[entryId * 2]);
    evtSetValue(event, args[2], (s32)hosi_name[entryId * 2 + 1]);
    return 2;
}
