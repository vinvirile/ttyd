#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void effSmallStarEntry(s32 kind, s32 extra, f32 x, f32 y, f32 z, f32 a, f32 b, f32 c);

static s32 small_star(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 x = evtGetFloat(event, args[0]);
    f32 y = evtGetFloat(event, args[1]);
    f32 z = evtGetFloat(event, args[2]);
    effSmallStarEntry(0, 4, x, y + 12.5f, z, 0.0f, -1.0f, 0.0f);
    return 2;
}
