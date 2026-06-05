#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

extern void togeMakeRetsu(Vec* vec, s32 a, s32 b, s32 c);
extern Vec vec3_mri_00023be0;

s32 toge_enter_func00(EventEntry* event, BOOL isFirstCall) {
    Vec vec = vec3_mri_00023be0;
    togeMakeRetsu(&vec, 0x6e, 0x6f, 0x70);
    togeMakeRetsu(&vec, 0x71, 0x72, 0x73);
    togeMakeRetsu(&vec, 0x74, 0x75, 0x77);
    return 2;
}
