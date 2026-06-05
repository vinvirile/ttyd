#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern s32 kpaGetLevel(void);

static s32 kpa_get_level(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 level = kpaGetLevel();
    evtSetValue(event, args[0], level);
    return 2;
}
