#pragma once

#include <dolphin/types.h>
#include "driver/camdrv.h"
#include "driver/effdrv.h"

EffectEntry* effUranokoEntry(EffectEntry* effect, f32 x, f32 y, f32 z, s32 time);
void effUranokoMain(EffectEntry* effect);
void effUranokoDisp(CameraId camId, void* work);
