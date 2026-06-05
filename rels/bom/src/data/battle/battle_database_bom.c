#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void effSnowfallN64Entry(s32 a, s32 b);

static s32 _snow_control(EventEntry* event, BOOL isFirstCall) {
    if (isFirstCall) {
        effSnowfallN64Entry(0, 0x40);
    }
    return 0;
}
