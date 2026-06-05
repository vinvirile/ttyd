#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_sub.h"
#include "battle/battle_unit.h"
#include "battle/battle_event_cmd.h"

extern BattleWork* _battleWorkPointer;

static s32 kanbu_get_id(EventEntry* event, BOOL isFirstCall) {
    BattleWork* bp;
    s32* args;
    s32 foundId;
    s32 i;
    BattleWorkUnit* unit;
    bp = _battleWorkPointer;
    args = event->args;
    foundId = -1;
    i = 0;
    unit = NULL;
    for (i = 0; i < 64; i++) {
        unit = BattleGetUnitPtr(bp, i);
        if (unit != NULL && unit->mInitialKind == 0x63) {
            foundId = i;
            break;
        }
    }
    evtSetValue(event, args[0], foundId);
    if (foundId != -1) {
        evtSetValue(event, args[1], BtlUnit_GetBodyPartsId(unit));
    } else {
        evtSetValue(event, args[1], 0);
    }
    return 2;
}
