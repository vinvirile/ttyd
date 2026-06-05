#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void* evtNpcNameToPtr(EventEntry* event, char* name);
extern s32 evtGetValue(EventEntry* event, s32 arg);
extern f32 intplGetValue(s32 type, s32 val, f32 a, f32 b);
extern void* gp;

extern char str_00001DA0[];
extern f32 zero_gon_00005b68;
extern f32 float_360_gon_00005b6c;

static s32 evt_chuchu_back_loop(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    void* npc = evtNpcNameToPtr(event, str_00001DA0);
    s32 val = evtGetValue(event, args[0]);
    u32 timeLow;
    u32 timeHigh;
    u64 timeDiff;
    u32 divisor;

    if (*(s32*)((char*)gp + 0x14)) {
        timeLow = *(u32*)((char*)gp + 0x38);
        timeHigh = *(u32*)((char*)gp + 0x3c);
    } else {
        timeLow = *(u32*)((char*)gp + 0x40);
        timeHigh = *(u32*)((char*)gp + 0x44);
    }

    if (isFirstCall) {
        event->userdata[0] = timeLow;
        event->userdata[1] = timeHigh;
    }

    timeDiff = (u64)(timeHigh - event->userdata[1]);
    divisor = (*(u32*)0x800000F8 >> 2) / 1000;

    if ((u32)(timeDiff / divisor) < (u32)val) {
        *(f32*)((char*)npc + 0xf4) = intplGetValue(11, val, zero_gon_00005b68, float_360_gon_00005b6c);
        return 0;
    }

    *(f32*)((char*)npc + 0xf4) = zero_gon_00005b68;
    return 2;
}
