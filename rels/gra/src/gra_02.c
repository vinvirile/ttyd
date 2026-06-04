#include <dolphin/types.h>
#include "evt/evt_cmd.h"

extern NpcEntry* npcNameToPtr(const char* name);
extern void animPoseSetMaterialLightFlagOn(s32 poseId, s32 flags);

static char* booboo_name;

s32 booboo_main(void) {
    NpcEntry* npc = npcNameToPtr(booboo_name);
    animPoseSetMaterialLightFlagOn(npc->poseId, 0x10);
    return 2;
}
