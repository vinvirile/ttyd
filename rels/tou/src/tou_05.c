#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"

void gans_msg_callback(s32 msgType, void* param) {
    void* base = *(void**)param;
    EventEntry* event = *(EventEntry**)((char*)base + 0x7a00);
    if (msgType == 2) {
        s32 val = evtGetValue(event, 0xFD050F8A);
        evtSetValue(event, 0xFD050F8A, val + 1);
    }
}
