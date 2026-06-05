#include <dolphin/types.h>
#include <stdio.h>
#include "manager/evtmgr_cmd.h"

static char name[8];

static s32 make_name(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 val = evtGetValue(event, args[0]);
    sprintf(name, "hi%d", val);
    evtSetValue(event, args[1], (s32)name);
    return 2;
}
