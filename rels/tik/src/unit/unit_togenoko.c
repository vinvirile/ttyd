/* "unit_togenoko.c" - Togenoko battle unit (REL tik) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_sub.h"
#include "battle/battle_unit.h"
#include "battle/battle_event_cmd.h"

extern BattleWork* _battleWorkPointer;

static s32 togenoko_getnum(EventEntry* event, BOOL isFirstCall) {
    BattleWork* bp = _battleWorkPointer;
    s32* args = event->args;
    s32 i;
    s32 count = 0;
    s32 var;
    for (i = 0; i < 64; i++) {
        BattleWorkUnit* unit = BattleGetUnitPtr(bp, i);
        if (unit != NULL && unit->currentKind == 0x0B) {
            count++;
        }
    }
    evtSetValue(event, args[0], count);
    return 2;
}
