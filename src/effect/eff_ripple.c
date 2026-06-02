#include "eff/eff_ripple.h"
#include <string.h>

//.rodata
static const char str_ripple[] = "RippleN64";

//.sdata2
static const f32 float_0 = 0.0f;

void effRippleEntry(void* obj, f32 p1, f32 p2, f32 p3) {
    effRippleN64Entry(obj, p1, p2, p3, float_0);
}

void effRippleSetRxRz(void) {
    effRippleN64SetRxRz();
}

void effRippleSetCamId(void* obj, s32 camId) {
    void* data = *(void**)((char*)obj + 0xC);
    *(s32*)((char*)data + 0x34) = camId;
}

void effRippleSetPosition(void* obj, f32 x, f32 y, f32 z) {
    if (strcmp(*(char**)((char*)obj + 0x14), str_ripple) == 0) {
        *(f32*)((char*)*(void**)((char*)obj + 0xC) + 0x4) = x;
        *(f32*)((char*)*(void**)((char*)obj + 0xC) + 0x8) = y;
        *(f32*)((char*)*(void**)((char*)obj + 0xC) + 0xC) = z;
    }
}
