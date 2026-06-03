#include <dolphin/types.h>

typedef struct SND_HOOKS {
    void* (*malloc)(u32 len);
    void* (*mallocPhys)(u32 len, struct SND_HOOKS* hooks);
    void (*free)(void* addr);
} SND_HOOKS;

extern SND_HOOKS salHooks;

void* salMalloc(u32 len) { return salHooks.malloc(len); }

void* salMallocPhysical(u32 len) {
    return salHooks.mallocPhys(len, &salHooks);
}

void salFree(void* addr) { salHooks.free(addr); }
