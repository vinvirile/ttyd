/* "unit_hyper_jyugem.c" - Hyper Jyugem battle unit (REL jon) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_sub.h"
#include "battle/battle_unit.h"
#include "battle/battle_event_cmd.h"

extern BattleWork* _battleWorkPointer;

static s32 togezo_copy_status(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 id1 = BattleTransID(event, evtGetValue(event, args[0]));
    s32 id2 = BattleTransID(event, evtGetValue(event, args[1]));
    BattleWork* work = _battleWorkPointer;
    BattleWorkUnit* unit1 = BattleGetUnitPtr(work, id1);
    BattleWorkUnit* unit2 = BattleGetUnitPtr(work, id2);

    unit2->mSizeChangeTurns = unit1->mSizeChangeTurns;
    unit2->mSizeChangeStrength = unit1->mSizeChangeStrength;

    return 2;
}
