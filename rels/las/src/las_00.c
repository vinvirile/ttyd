#include <dolphin/types.h>
#include <stdio.h>
#include "manager/evtmgr_cmd.h"

static char name[0x10];

static s32 make_name(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    sprintf(name, "S_shan_rou_0%d", val);
    evtSetValue(event, args[1], (s32)name);
    return 2;
}
