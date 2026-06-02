#pragma once

#include <dolphin/types.h>

extern void effRippleN64SetRxRz(void);
extern void effRippleN64Entry(void* obj, f32 p1, f32 p2, f32 p3, f32 p4);

void effRippleSetCamId(void* obj, s32 camId);
void effRippleSetRxRz(void);
void effRippleEntry(void* obj, f32 p1, f32 p2, f32 p3);
void effRippleSetPosition(void* obj, f32 x, f32 y, f32 z);
