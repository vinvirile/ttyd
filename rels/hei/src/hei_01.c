#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "party.h"

static s32 set_dir(EventEntry* event, BOOL isFirstCall) {
    s32 val = evtGetValue(event, event->args[0]);
    PartyEntry* party = partyGetPtr(0);
    if (party != NULL) {
        s32 angle = (val > 0xB4) ? 0x10E : 0x5A;
        f32 rot = (f32)(0x10E - angle);
        *(f32*)((char*)party + 0x10C) = rot;
        *(f32*)((char*)party + 0x110) = rot;
    }
    return 2;
}
