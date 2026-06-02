#pragma once

#include <dolphin/types.h>

typedef struct MailEntry {
    u32 flags;     // 0x0
    s32 mailId;    // 0x4
    char* nameStr; // 0x8
    char* msgStr;  // 0xC
    u32 extra;     // 0x10
} MailEntry; // size 0x14

MailEntry* mailGetPtr(s32 mailId);
