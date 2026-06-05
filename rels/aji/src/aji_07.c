#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern f32 evtGetFloat(EventEntry* event, s32 arg);
extern void* marioGetPtr(void);
extern void* partyGetPtr(s32 id);

extern Vec vec3_aji_00012c6c;
extern Vec vec3_aji_00012c78;

static s32 scale_func(EventEntry* event, BOOL isFirstCall) {
    f32 scale = evtGetFloat(event, event->args[0]);
    void* mario = marioGetPtr();
    void* party = partyGetPtr(0);
    Vec vec;

    vec = vec3_aji_00012c6c;
    vec.x = scale;
    vec.y = scale;
    vec.z = scale;
    ((Vec*)((char*)mario + 0xc8))->x = vec.x;
    ((Vec*)((char*)mario + 0xc8))->y = vec.y;
    ((Vec*)((char*)mario + 0xc8))->z = vec.z;

    if (party != NULL) {
        vec = vec3_aji_00012c78;
        vec.x = scale;
        vec.y = scale;
        vec.z = scale;
        ((Vec*)((char*)party + 0x70))->x = vec.x;
        ((Vec*)((char*)party + 0x70))->y = vec.y;
        ((Vec*)((char*)party + 0x70))->z = vec.z;
    }

    return 2;
}
