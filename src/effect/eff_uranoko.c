#include "eff/eff_uranoko.h"
#include <string.h>

#include <dolphin/mtx.h>
#include <dolphin/types.h>

#include "driver/animdrv.h"
#include "driver/camdrv.h"
#include "driver/dispdrv.h"
#include "driver/effdrv.h"
#include "mario/mariost.h"
#include "memory.h"

extern GlobalWork* gp;
void animPoseDrawMtx(s32 poseId, Mtx matrix, s32 xluMode, f32 rotY, f32 scale);

typedef struct {
    EffectEntry* parent;  // 0x0
    f32 x;                // 0x4
    f32 y;                // 0x8
    f32 z;                // 0xC
    f32 scale;            // 0x10
    f32 unk14;            // 0x14
    s32 count;            // 0x18
    s32 timeout;          // 0x1C
    s32 poseId;           // 0x20
    u32 color;            // 0x24
} effUranokoData;

//.rodata
static const Vec vec3_80301390 = {0.0f, 0.0f, 0.0f};
static const char str_8030139C[] = "MOBJ_EFF_uranoko\0\0\0";
static const char str_803013B0[] = "Uranoko";

//.sdata2
static const char str_80422328[] = "A_1";
static const u32 dat_80422318 = 0xFFFFFF00;
static const f32 float_deg2rad_8042231c = 0.017453292f;
static const f32 float_0_80422320 = 0.0f;
static const f32 float_6_80422324 = 6.0f;
static const f32 float_1_8042232c = 1.0f;
static const f32 float_0p125_80422330 = 0.125f;
static const f32 float_gap_80422334 = 0.0f;  // gap_09_80422334_sdata2

EffectEntry* effUranokoEntry(EffectEntry* effect, f32 x, f32 y, f32 z, s32 time) {
    EffectEntry* entry;
    void* data;
    effUranokoData* d;
    entry = effEntry();
    strcpy(entry->name, str_803013B0);
    entry->count = 1;
    data = __memAlloc(3, entry->count * 0x28);
    entry->userdata = data;
    entry->callback = effUranokoMain;
    d = data;
    d->parent = effect;
    d->x = x;
    d->y = y;
    d->z = z;
    d->scale = float_0_80422320;
    d->unk14 = float_1_8042232c;
    d->count = 0;
    d->timeout = (time > 0) ? time : 0x3E8;
    d->poseId = -1;
    d->color = 0xFF;
    effect->flags |= 0x2;
    return entry;
}

void effUranokoMain(EffectEntry* effect) {
    effUranokoData* d;
    Vec vec;
    s32 group;
    d = effect->userdata;
    vec = vec3_80301390;
    vec.x = d->x;
    vec.y = d->y;
    vec.z = d->z;
    group = (gp->inBattle != 0);
    if (animGroupBaseAsync(str_8030139C, group, NULL) != 0) {
        if (d->poseId == -1) {
            d->poseId = animPoseEntry(str_8030139C, group);
            animPoseSetAnim(d->poseId, str_80422328, TRUE);
        }
        if (effect->flags & 0x4) {
            effect->flags &= ~0x4;
            d->timeout = 0x10;
        }
        if (d->timeout < 0x3E8) {
            d->timeout--;
        }
        if (d->timeout < 0x10) {
            d->color = d->timeout << 4;
        }
        if (d->timeout < 0) {
            if (d->poseId != -1) {
                animPoseRelease(d->poseId);
            }
            effDelete(effect);
            return;
        }
        d->count++;
        d->scale = (1.0f - d->scale) * float_0p125_80422330 + d->scale;
        dispCalcZ(&vec);
        dispEntry(CAMERA_3D, 2, effUranokoDisp, effect, d->count);
    }
}

void effUranokoDisp(CameraId camId, void* work) {
    effUranokoData* d;
    Mtx mtxTrans, mtxScale, mtxRot;
    GXColor color;
    d = work;
    if (d->poseId == -1) return;
    PSMTXTrans(mtxTrans, d->x, d->y, d->z);
    PSMTXScale(mtxScale, d->scale * d->unk14, d->scale * d->unk14, d->scale * d->unk14);
    PSMTXRotRad(mtxRot, 'y', float_deg2rad_8042231c * -camGetPtr(CAMERA_3D)->viewYaw);
    PSMTXConcat(mtxTrans, mtxRot, mtxTrans);
    PSMTXConcat(mtxTrans, mtxScale, mtxTrans);
    animPoseSetMaterialFlagOn(d->poseId, 0x40);
    *(u32*)&color = dat_80422318;
    color.a = d->color;
    animPoseSetMaterialEvtColor(d->poseId, &color);
    animPoseMain(d->poseId);
    animPoseDrawMtx(d->poseId, mtxTrans, 1, float_0_80422320, float_6_80422324);
    animPoseDrawMtx(d->poseId, mtxTrans, 2, float_0_80422320, float_6_80422324);
    animPoseDrawMtx(d->poseId, mtxTrans, 3, float_0_80422320, float_6_80422324);
}
