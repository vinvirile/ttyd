#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "driver/npcdrv.h"

extern s32 winGhostDiaryChk(void);

static s32 mario_flag(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    FieldBattleData* fbat = fbatGetPointer();
    if (*(s16*)&fbat->mode == 2 || winGhostDiaryChk()) {
        evtSetValue(event, args[0], 1);
    } else {
        evtSetValue(event, args[0], 0);
    }
    return 2;
}
