#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "driver/effdrv.h"

extern EffectEntry* effGuruguruN64Entry(s32 kind, s32 extra, f32 x, f32 y, f32 z, f32 w);

static s32 eff_guruguru(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 name = evtGetValue(event, args[0]);
    f32 x = evtGetFloat(event, args[1]);
    f32 y = evtGetFloat(event, args[2]);
    f32 z = evtGetFloat(event, args[3]);
    f32 w = evtGetFloat(event, args[4]);
    EffectEntry* eff = effGuruguruN64Entry(0, 3, x, y, z, w);
    *(f32*)((char*)eff->userdata + 0x2C) = 0.8f;
    effSetName(eff, (const char*)name);
    return 2;
}
