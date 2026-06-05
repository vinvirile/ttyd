#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void* evtNpcNameToPtr(EventEntry* event, char* name);
extern s32 evtGetValue(EventEntry* event, s32 arg);
extern f32 intplGetValue(s32 type, s32 val, f32 a, f32 b);
extern void* gp;

extern char str_00002708[];
extern f32 zero_mri_00022a38;
extern f32 float_360_mri_00022a3c;

static s32 evt_chuchu_back_loop(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    void* npc = evtNpcNameToPtr(event, str_00002708);
    s32 val = evtGetValue(event, args[0]);

    if (isFirstCall) {
        event->userdata[0] = *(s32*)((char*)gp + 0x3c);
    }

    if ((*(u32*)((char*)gp + 0x3c) - event->userdata[0]) / ((*(u32*)0x800000F8 >> 2) / 1000) < (u32)val) {
        *(f32*)((char*)npc + 0xf4) = intplGetValue(11, val, zero_mri_00022a38, float_360_mri_00022a3c);
        return 0;
    }

    *(f32*)((char*)npc + 0xf4) = zero_mri_00022a38;
    return 2;
}
