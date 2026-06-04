#include "battle/ac/ac_crane_timing.h"

void battleAcDelete_CraneTiming(BattleWork* work) {
    *(s32*)((u8*)work + 0x1C9C) = 0x3EA;
}

void battleAcDisp_CraneTiming(void) {
}

s32 battleAcResult_CraneTiming(BattleWork* work) {
    return *(s32*)((u8*)work + 0x1CB8);
}

#pragma dont_inline on
s32 battleAcMain_CraneTiming(BattleWork* work) {
    // TODO: decompile 1076B function
    return 0;
}
#pragma dont_inline off
