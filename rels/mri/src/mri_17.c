#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "mario/mariost.h"
#include "driver/imgdrv.h"

extern GlobalWork* gp;

static s32 set_depthPlaneOffset(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    const char* name = (const char*)evtGetValue(event, args[0]);
    ImageEntry* img = imgNameToPtr(name, (gp->inBattle != 0) ? 1 : -1);
    img->field_0x11C = evtGetFloat(event, args[1]);
    return 2;
}
