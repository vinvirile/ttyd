#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void effKemuri9N64Entry(s32 kind, s32 unk1, s32 unk2, f32 x, f32 y, f32 z, f32 scale, f32 unk3);

static s32 _leg_smoke_effect(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 x = evtGetFloat(event, args[0]);
    f32 y = evtGetFloat(event, args[1]);
    f32 z = evtGetFloat(event, args[2]);
    effKemuri9N64Entry(1, 0x12, 0x5a, x, y, z, 60.0f, 0.0f);
    return 2;
}
