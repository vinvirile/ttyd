#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void* marioGetPtr(void);
extern void marioChgPose(s32 pose);

static s32 peach_set_pose(EventEntry* event, BOOL isFirstCall) {
    s32 pose = evtGetValue(event, event->args[0]);
    marioGetPtr();
    marioChgPose(pose);
    return 2;
}
