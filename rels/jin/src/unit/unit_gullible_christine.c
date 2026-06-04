/* "unit_gullible_christine.c" - Gullible Christine battle unit (REL jin) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_event_cmd.h"
#include "system.h"

extern f32 angleABf(f32, f32, f32, f32);

USER_FUNC(_krb_get_dir);

static const f32 zero_jin = 0.0f;

s32 _krb_get_dir(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 dx = evtGetFloat(event, args[0]);
    f32 dy = evtGetFloat(event, args[1]);
    f32 px = evtGetFloat(event, args[2]);
    f32 py = evtGetFloat(event, args[3]);
    s32 out = evtGetValue(event, args[4]);

    if (px - dx == 0.0f && py - dy == 0.0f) {
        evtSetValue(event, args[4], out);
        return 2;
    }

    evtSetValue(event, args[4], (s32)angleABf(px - dx, py - dy, 0.0f, 0.0f));
    return 2;
}
