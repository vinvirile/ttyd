#include <dolphin/types.h>
#include "manager/evtmgr_cmd.h"
#include "driver/hitdrv.h"

extern void* marioGetPtr(void);
extern int strcmp(const char*, const char*);

extern char* hitname[];
extern char* mapanim[];

static s32 mario_moveride_check(EventEntry* event, BOOL isFirstCall) {
    s32* args;
    void* mario;
    void* hit;
    char** htbl;
    s32 matchedHit;
    s32 i;
    s32 idx;
    s32 matchedAnim;

    args = event->args;
    mario = marioGetPtr();
    hit = *(void**)((char*)mario + 0x1e4);
    htbl = hitname;
    matchedHit = 0;
    i = 0;
    idx = 0;

    for (i = 0; i < 17; i++) {
        if (hit != NULL) {
            if (strcmp(hitGetName(hit), htbl[i]) == 0) {
                matchedHit = (s32)htbl[i];
                matchedAnim = (s32)mapanim[i];
                break;
            }
        }
    }

    event->userdata[9] = 0;
    if (matchedHit != 0) {
        if ((s32)event->userdata[10] != matchedHit) {
            event->userdata[9] = matchedAnim;
        }
    }
    event->userdata[10] = matchedHit;
    return 2;
}
