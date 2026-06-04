/* "unit_ura_noko.c" - Ura Noko battle unit (REL jon) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_unit.h"
#include "eff/eff_uranoko.h"

extern BattleWork* _battleWorkPointer;

s32 eff_aura(EventEntry* event, BOOL isFirstCall) {
    BattleWorkUnit* unit;
    f32 x, y, z;
    EffectEntry* aura;
    f32* data;

    unit = BattleGetUnitPtr(_battleWorkPointer, BattleTransID(event, evtGetValue(event, event->args[0])));

    if (isFirstCall) {
        unit->work[2] = (s32)effUranokoEntry(NULL, 0.0f, -1000.0f, 0.0f, 0x78);
    }

    BtlUnit_GetPos(unit, &x, &y, &z);

    aura = (EffectEntry*)unit->work[2];
    if (!(aura->flags & 1)) {
        return 2;
    }

    data = (f32*)aura->userdata;
    data[1] = x;
    data[2] = y;
    data[3] = z;
    data[5] = unit->mSizeMultiplier;

    return 0;
}
