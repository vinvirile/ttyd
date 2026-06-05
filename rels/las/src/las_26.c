#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "driver/npcdrv.h"

static s32 bonbaba_wait_landon(EventEntry* event, BOOL isFirstCall) {
    NpcEntry* npc = npcNameToPtr("\203{\203\223\203o\203o");
    return (*(s32*)&npc->field_0x2FC[4] != 0) ? 2 : 0;
}
