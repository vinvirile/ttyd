/* "unit_gold_chorobon.c" - Gold Chorobon battle unit (REL hei) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_unit.h"

extern BattleWork* _battleWorkPointer;

s32 get_gold_chorobon_id(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    BattleWork* work = _battleWorkPointer;
    s32 i;

    for (i = 0; i < 64; i++) {
        BattleWorkUnit* unit = BattleGetUnitPtr(work, i);
        if (unit != NULL && unit->mInitialKind == 0x14) {
            break;
        }
    }

    if (i != 64) {
        evtSetValue(event, args[0], i);
    } else {
        evtSetValue(event, args[0], -1);
    }
    return 2;
}
