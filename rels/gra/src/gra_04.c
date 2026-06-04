#include <dolphin/types.h>
#include "evt/evt_cmd.h"

extern BOOL unk_8009fcbc(void);

s32 unk_gra_00000530(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 result = unk_8009fcbc();
    evtSetValue(event, args[0], !result);
    return EVT_RETURN_DONE;
}
