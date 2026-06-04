/* "unit_borodo.c" - Borodo battle unit (REL jon) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_sub.h"
#include "battle/battle_unit.h"
#include "battle/battle_event_cmd.h"
#include "mario/mario_pouch.h"
#include "system.h"

extern BattleWork* _battleWorkPointer;
extern BOOL BtlUnit_EquipItem(BattleWorkUnit*, s32, s32);
extern const s32 itemDataTable[];

USER_FUNC(_steal_item);

s32 _steal_item(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    BattleWork* work;
    PouchData* pouch;
    s32 arg2, arg3, arg4;
    s32 r3;
    BattleWorkUnit* unit1;
    BattleWorkUnit* unit2;
    s32 coin, amount, count, itemId, i, kind;

    work = _battleWorkPointer;
    pouch = pouchGetPtr();
    arg2 = args[2];
    arg3 = args[3];
    arg4 = args[4];

    unit1 = BattleGetUnitPtr(work, BattleTransID(event, evtGetValue(event, args[0])));
    if (!unit1) return 2;

    unit2 = BattleGetUnitPtr(work, BattleTransID(event, evtGetValue(event, args[1])));
    if (!unit2) return 2;

    evtSetValue(event, arg2, 0);
    evtSetValue(event, arg3, 0);
    evtSetValue(event, arg4, 0);

    r3 = irand(100);
    if (r3 < 100) {
        coin = pouchGetCoin();
        amount = 5;
        if (coin < 5) amount = coin;
        if (amount <= 0) return 2;
        evtSetValue(event, arg2, 0x79);
        evtSetValue(event, arg3, 0x18b);
        evtSetValue(event, arg4, amount);
        unit1->heldItem = 0x79;
        unit1->field_0x308[4] = (u8)amount;
        pouchSetCoin((s16)(coin - amount));
        return 2;
    } else {
        count = 0;
        itemId = 0;

        for (i = 0; i < 20; i++) {
            if (pouch->heldItems[i] != 0) count++;
        }
        if (count <= 0) return 2;

        r3 = irand(count);
        if (r3 < count) {
            for (i = 0; i < 20; i++) {
                if (pouch->heldItems[i] != 0) {
                    r3--;
                    if (r3 < 0) {
                        itemId = pouch->heldItems[i];
                        break;
                    }
                }
            }
        } else {
            s16* badges = (s16*)((char*)pouch + 0x1fa);
            for (i = 0; i < 200; i++) {
                if (badges[i] != 0) {
                    r3--;
                    if (r3 < 0) {
                        itemId = badges[i];
                        break;
                    }
                }
            }
        }

        if (itemId == 0) return 2;
        if (!pouchRemoveItem(itemId)) return 2;

        unit1->heldItem = itemId;
        if (itemId >= 0xf0 && itemId < 0x153) {
            kind = unit2->currentKind;
            if (kind == 0xde) {
                BtlUnit_EquipItem(unit2, 3, 0);
            } else if (kind >= 0xe0 && kind < 0xe7) {
                BtlUnit_EquipItem(unit2, 5, 0);
            }
            pouchReviseMarioParam();
            pouchRevisePartyParam();
        }

        evtSetValue(event, arg2, itemId);
        evtSetValue(event, arg3, *(s16*)((char*)itemDataTable + itemId * 0x28 + 0x20));
    }
    return 2;
}
