/* "unit_boss_gesso_right_arm.c" - Gesso right arm (REL tik) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_sub.h"
#include "battle/battle_unit.h"
#include "battle/battle_event_cmd.h"

extern BattleWork* _battleWorkPointer;

static s32 _check_zenei_kouei(EventEntry* event, BOOL isFirstCall) {
    BattleWork* bp = _battleWorkPointer;
    s32* args = event->args;
    s32 id1 = evtGetValue(event, args[0]);
    s32 id2 = BattleTransID(event, id1);
    s32 outIdx = args[1];
    f32 x, y, z;
    BattleWorkUnit* unit = BattleGetUnitPtr(bp, id2);
    BtlUnit_GetPos(unit, &x, &y, &z);
    if (x >= -120.0f) {
        evtSetValue(event, outIdx, 0);
    } else {
        evtSetValue(event, outIdx, 1);
    }
    return 2;
}
