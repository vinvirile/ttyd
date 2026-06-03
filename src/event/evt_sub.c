/* "evt_sub.c" - Event utility functions */

#include "evt/evt_sub.h"
#include "manager/evtmgr.h"
#include "mario/mariost.h"
#include "driver/swdrv.h"
#include "mario/mario.h"
#include "mario/mario_pouch.h"
#include "driver/camdrv.h"
#include "driver/effdrv.h"
#include "driver/icondrv.h"
#include "driver/npcdrv.h"
#include "driver/msgdrv.h"
#include "manager/winmgr.h"
#include "manager/fontmgr.h"
#include "statuswindow.h"
#include "pmario_sound.h"
#include "system.h"
#include "memory.h"
#include <dolphin/os.h>
#include <math.h>
#include <string.h>

extern GlobalWork* gp;

extern s32 marioCtrlOffChk(void);
extern s32 marioKeyOffChk(void);
extern void marioChgGetItemMotion(void);
extern void marioChgStayMotion(void);
extern void marioSetCamId(s32 camId);
extern MarioWork* marioGetPtr(void);
extern void countDownSetRestartTime(s32 time);
extern void countDownFlagOn(s32 flag);
extern void countDownFlagOff(s32 flag);
extern void countDownEnd(void);
extern s32 countDownGetStatus(void);
extern void countDownStart(s32 time, s32 flag);
extern void statusWinForceOpen(void);
extern void statusWinForceClose(void);
extern s32 statusWinCheckUpdate(void);
extern EffectEntry* effItemGetEntry(s32 id);
extern void effStageClearEndChk(EffectEntry* eff);
extern void* itemNameToPtr(const char* name);
extern void FontDrawColor(s32 r, s32 g, s32 b);
extern s32 FontGetMessageWidth(void);
extern void FontDrawMessage(const char* msg);
extern void spline_getvalue(void* table, f32 time, void* work, void* knots, f32* out);
extern void spline_maketable(s32 count, void* table, void* work, void* knots);
extern f32 angleABf(f32 x1, f32 y1, f32 x2, f32 y2);
extern s32 evt_run_case_evt(EventEntry* event, s32 arg1, s32 arg2, void* data);

extern s32 stop;

typedef struct SplineData {
    s32 count;       /* 0x00 */
    void* dataBuf;   /* 0x04 */
    void* knots;     /* 0x08 */
    void* workBuf;   /* 0x0C */
    s32 frameCount;  /* 0x10 */
    s32 frameTotal;  /* 0x14 */
    s32 unk18;       /* 0x18 */
    s32 unk1C;       /* 0x1C */
    s32 savedArg0;   /* 0x20 */
    s32 savedArg1;   /* 0x24 */
    s32 timeEnd;     /* 0x28 */
} SplineData;

/* lwData offsets used for intpl state:
 * lwData[11] = event+0xC8 = mode
 * lwData[12] = event+0xCC = start
 * lwData[13] = event+0xD0 = end
 * lwData[14] = event+0xD4 = step
 * lwData[15] = event+0xD8 = total / spline ptr
 */

#define INTPL_MODE(e)   ((e)->lwData[11])
#define INTPL_START(e)  ((e)->lwData[12])
#define INTPL_END(e)    ((e)->lwData[13])
#define INTPL_STEP(e)   ((e)->lwData[14])
#define INTPL_TOTAL(e)  ((e)->lwData[15])
#define SPLINE_PTR(e)   (*(SplineData**)&(e)->lwData[15])

static s32 bgm_start_wait(EventEntry* event, BOOL isFirstCall) {
    if (psndBGMStartCheck(1) != 0) {
        return 2;
    }
    return 0;
}

static s32 get_stop(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], stop);
    return 2;
}

static s32 set_stop(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    stop = evtGetValue(event, args[0]);
    return 2;
}

static s32 stageclear_wait(EventEntry* event, BOOL isFirstCall) {
    EffectEntry* eff = effNameToPtr("stageclear");
    effStageClearEndChk(eff);
    return eff != NULL ? 2 : 0;
}

static s32 stone_bg_open(EventEntry* event, BOOL isFirstCall) {
    EffectEntry* eff = effNameToPtr("stone_bg");
    s32* sub = (s32*)eff->userdata;
    sub[8] = 1;
    return 2;
}

static s32 stone_bg(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 itemId = evtGetValue(event, args[0]);
    CameraEntry* cam = camGetPtr(8);
    EffectEntry* eff;
    void* item;
    NpcEntry* npc;

    cam->mode = 5;

    eff = effNameToPtr("stone_bg");
    if (eff != NULL) {
        s32* sub = (s32*)((char*)eff->userdata + 0x0C);
        sub[0x38 / 4] = 5;
    }

    item = itemNameToPtr((const char*)itemId);
    if (item != NULL) {
        s32* sub = (s32*)((char*)item + 0x0C);
        sub[0x38 / 4] = 5;
    }

    npc = npcNameToPtr_NoAssert("pik");
    if (npc != NULL) {
        s32 val = (s32)evtGetValue(event, (s32)0xF5DE0180);
        if (val < 0xD3) {
            npc->camId = 5;
        } else {
            marioSetCamId(5);
        }
    } else {
        marioSetCamId(5);
    }

    return 2;
}

static s32 stone_ry(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 itemId = evtGetValue(event, args[0]);
    s32 rotation = evtGetValue(event, args[1]);
    void* item = itemNameToPtr((const char*)itemId);
    if (item != NULL) {
        u32* flags = (u32*)((char*)item + 0x04);
        *flags &= ~0x3;
        *(f32*)((char*)item + 0x1C) = (f32)rotation;
    }
    return 2;
}

s32 evt_sub_get_coin(EventEntry* event, BOOL isFirstCall) {
    return 2;
}

s32 coingetDisp(EventEntry* event, BOOL isFirstCall) {
    return 2;
}

s32 evt_sub_countdown_set_restart_time(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    countDownSetRestartTime(evtGetValue(event, args[0]));
    return 2;
}

s32 evt_sub_countdown_flag_onoff(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 onoff = evtGetValue(event, args[0]);
    s32 flag = evtGetValue(event, args[1]) & 0xFFFF;
    if (onoff != 0) {
        countDownFlagOn(flag);
    } else {
        countDownFlagOff(flag);
    }
    return 2;
}

s32 evt_sub_countdown_end(EventEntry* event, BOOL isFirstCall) {
    countDownEnd();
    return 2;
}

s32 evt_sub_countdown_get_status(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], countDownGetStatus());
    return 2;
}

s32 evt_sub_countdown_start(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 time = evtGetValue(event, args[0]);
    s32 flag = evtGetValue(event, args[1]);
    countDownStart(time, flag);
    return 2;
}

s32 evt_sub_animgroup_async(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    const char* name = (const char*)evtGetValue(event, args[0]);
    s32 inBattle = gp->inBattle;
    if (animGroupBaseAsync(name, inBattle != 0 ? 1 : 0, NULL) == 0) {
        return 0;
    }
    return 2;
}

s32 evt_sub_get_language(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], gp->language);
    return 2;
}

s32 evt_sub_load_progresstime(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 switchId = evtGetValue(event, args[0]);
    u8 data[8];
    s32 i;
    OSTime saved, now;
    s32 elapsed;

    for (i = 0; i < 8; i++) {
        data[i] = swByteGet(switchId + 0xA22FE80 + i);
    }

    saved = ((OSTime)data[0] << 56) | ((OSTime)data[1] << 48) |
            ((OSTime)data[2] << 40) | ((OSTime)data[3] << 32) |
            ((OSTime)data[4] << 24) | ((OSTime)data[5] << 16) |
            ((OSTime)data[6] << 8) | (OSTime)data[7];

    now = OSGetTime();
    elapsed = (s32)((now - saved) / OS_TIMER_CLOCK);
    if (elapsed > 0x05265C00) {
        elapsed = 0x05265C00;
    }
    if (elapsed < 0) {
        elapsed = 0;
    }
    evtSetValue(event, args[1], elapsed);
    return 2;
}

s32 evt_sub_save_playtime(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 switchId = evtGetValue(event, args[0]);
    OSTime time = gp->saveTime;
    s32 i;

    for (i = 0; i < 8; i++) {
        swByteSet(switchId + 0xA22FE80 + i, (s32)(u8)(time >> (56 - i * 8)));
    }
    return 2;
}

s32 evt_sub_status_gauge_check_update(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], statusWinCheckUpdate() != 0 ? 1 : 0);
    return 2;
}

s32 evt_sub_status_gauge_force_close(EventEntry* event, BOOL isFirstCall) {
    statusWinForceClose();
    return 2;
}

s32 evt_sub_status_gauge_force_open(EventEntry* event, BOOL isFirstCall) {
    statusWinForceOpen();
    return 2;
}

s32 evt_sub_get_areaname(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], (s32)gp->currentAreaName);
    return 2;
}

s32 evt_sub_get_mapname(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], (s32)gp->currentMapName);
    return 2;
}

s32 evt_key_get_buttontrg(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    u32 port = evtGetValue(event, args[0]);
    evtSetValue(event, args[1], keyGetButtonTrg(port));
    return 2;
}

s32 evt_key_get_button(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    u32 port = evtGetValue(event, args[0]);
    evtSetValue(event, args[1], keyGetButton(port));
    return 2;
}

s32 evt_key_get_dir(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    u32 port = evtGetValue(event, args[0]);
    evtSetValue(event, args[1], keyGetDir(port));
    return 2;
}

s32 evt_sub_system_flag_onoff(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 onoff = evtGetValue(event, args[0]);
    s32 mask = evtGetValue(event, args[1]);
    if (onoff != 0) {
        gp->flags |= mask;
    } else {
        gp->flags &= ~mask;
    }
    return 2;
}

s32 evt_sub_get_system_flag(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], gp->flags);
    return 2;
}

s32 evt_sub_get_dir(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 x1 = (f32)evtGetValue(event, args[0]);
    f32 y1 = (f32)evtGetValue(event, args[1]);
    f32 x2 = (f32)evtGetValue(event, args[2]);
    f32 y2 = (f32)evtGetValue(event, args[3]);
    evtSetFloat(event, args[4], angleABf(x1, y1, x2, y2));
    return 2;
}

s32 evt_sub_get_dist(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 x1 = (f32)evtGetValue(event, args[0]);
    f32 y1 = (f32)evtGetValue(event, args[1]);
    f32 x2 = (f32)evtGetValue(event, args[2]);
    f32 y2 = (f32)evtGetValue(event, args[3]);
    evtSetFloat(event, args[4], distABf(x1, y1, x2, y2));
    return 2;
}

s32 evt_sub_area_check(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 type = evtGetValue(event, args[0]);
    f32 cx = (f32)evtGetValue(event, args[1]);
    f32 cy = (f32)evtGetValue(event, args[2]);
    f32 r = (f32)evtGetValue(event, args[3]);
    f32 px = (f32)evtGetValue(event, args[4]);
    f32 py = (f32)evtGetValue(event, args[5]);
    s32 result = 0;

    if (type == 0) {
        if (distABf(cx, cy, px, py) >= r) {
            result = 1;
        }
    } else if (type == 1) {
        if (cx - r <= px && px <= cx + r && cy - r <= py && py <= cy + r) {
            result = 1;
        }
    }
    evtSetValue(event, args[8], result);
    return 2;
}

s32 evt_sub_get_stopwatch(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 ptrId = evtGetValue(event, args[0]);
    EventEntry* ptr = evtGetPtrID(ptrId);
    s32 elapsed;
    s32 maxMs = 0x927C0;

    elapsed = (s32)(ptr->lifetime / (OSTime)OS_TIMER_CLOCK);
    if (elapsed > maxMs) {
        elapsed = maxMs;
    }
    evtSetValue(event, args[1], elapsed);
    return 2;
}

s32 evt_sub_random(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 max = evtGetValue(event, args[0]);
    s32 val = rand();
    evtSetValue(event, args[1], val - (val / (max + 1)) * (max + 1));
    return 2;
}

s32 evt_sub_rumble_onoff(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 mode = evtGetValue(event, args[0]);
    s32 chan = evtGetValue(event, args[1]);
    switch (mode) {
    case 0:
        padRumbleOn(chan);
        break;
    case 1:
        padRumbleOff(chan);
        break;
    case 2:
        padRumbleHardOff(chan);
        break;
    }
    return 2;
}

s32 evt_sub_get_sincos(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 angle = evtGetFloat(event, args[0]);
    f32 radians = angle * 3.14159265f / 180.0f;
    s32 sinVal = (s32)(sinf(radians) * 1024.0f) - 0x0DB58580;
    s32 cosVal = (s32)(cosf(radians) * 1024.0f) - 0x0DB58580;
    evtSetValue(event, args[1], sinVal);
    evtSetValue(event, args[2], cosVal);
    return 2;
}

s32 evt_sub_spline_free(EventEntry* event, BOOL isFirstCall) {
    SplineData* spline = SPLINE_PTR(event);
    if (spline != NULL) {
        _mapFree(spline->dataBuf);
        _mapFree(spline->workBuf);
        _mapFree(spline);
        SPLINE_PTR(event) = NULL;
    }
    return 2;
}

s32 evt_sub_spline_get_value_manual(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    SplineData* spline = SPLINE_PTR(event);
    f32 out[3];
    f32 time = (f32)evtGetValue(event, args[0]);
    f32 divisor;

    if (spline->timeEnd != 0) {
        divisor = (f32)spline->timeEnd;
    } else {
        divisor = (f32)spline->frameTotal;
    }
    time = time / divisor;

    spline_getvalue(spline->dataBuf, time, spline->workBuf, spline->knots, out);
    evtSetValue(event, args[1], (s32)(out[0] * 1024.0f));
    evtSetValue(event, args[2], (s32)(out[1] * 1024.0f));
    evtSetValue(event, args[3], (s32)(out[2] * 1024.0f));
    return 2;
}

s32 evt_sub_spline_get_value(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    SplineData* spline = SPLINE_PTR(event);
    f32 out[3];
    f32 time;
    OSTime elapsed;
    s32 done = 0;

    elapsed = OSGetTime() - event->lifetime;
    if (spline->timeEnd != 0) {
        time = (f32)((s32)(elapsed / (OSTime)OS_TIMER_CLOCK)) / (f32)spline->timeEnd;
    } else {
        time = (f32)((s32)(elapsed / (OSTime)OS_TIMER_CLOCK)) / (f32)spline->frameTotal;
    }

    spline_getvalue(spline->dataBuf, time, spline->workBuf, spline->knots, out);
    evtSetValue(event, args[0], (s32)(out[0] * 1024.0f));
    evtSetValue(event, args[1], (s32)(out[1] * 1024.0f));
    evtSetValue(event, args[2], (s32)(out[2] * 1024.0f));

    spline->frameCount += 1;

    if (spline->timeEnd != 0) {
        if ((s32)(elapsed / (OSTime)OS_TIMER_CLOCK) >= spline->timeEnd) {
            done = 1;
        }
    } else {
        if (spline->frameCount > spline->frameTotal) {
            done = 1;
        }
    }

    if (done) {
        evtSetValue(event, args[3], 1);
        _mapFree(spline->dataBuf);
        _mapFree(spline->workBuf);
        _mapFree(spline);
        SPLINE_PTR(event) = NULL;
        return 2;
    }

    return 0;
}

s32 evt_sub_spline_init(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 count = evtGetValue(event, args[0]);
    s32 arg1 = evtGetValue(event, args[1]);
    s32 arg2 = evtGetValue(event, args[2]);
    s32 arg3 = evtGetValue(event, args[3]);
    s32 arg4 = evtGetValue(event, args[4]);
    s32 arg5 = evtGetValue(event, args[5]);
    s32 arg6 = evtGetValue(event, args[6]);
    SplineData* spline;

    spline = _mapAlloc(sizeof(SplineData));
    SPLINE_PTR(event) = spline;
    spline->count = count;
    spline->dataBuf = _mapAlloc(count * 4);
    spline->knots = (void*)arg2;
    spline->workBuf = _mapAlloc(count * 0xC);
    spline_maketable(count, spline->knots, spline->dataBuf, spline->workBuf);
    spline->timeEnd = arg1;
    spline->frameCount = 0;
    spline->frameTotal = arg3;
    spline->savedArg0 = event->args[0];
    spline->savedArg1 = event->args[1];
    return 2;
}

s32 evt_sub_intpl_msec_get_value_para(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32* argsPtr = event->args;
    f32 fStart, fEnd, result;

    if (INTPL_STEP(event) < INTPL_TOTAL(event)) {
        fStart = (f32)INTPL_START(event);
        fEnd = (f32)INTPL_END(event);
        result = intplGetValue(INTPL_MODE(event), INTPL_STEP(event), INTPL_TOTAL(event), fStart, fEnd);
        evtSetFloat(event, argsPtr[0], result);
        evtSetValue(event, argsPtr[1], 1);
    } else {
        fStart = (f32)INTPL_END(event);
        result = intplGetValue(INTPL_MODE(event), INTPL_STEP(event), INTPL_TOTAL(event), fStart, fStart);
        evtSetFloat(event, argsPtr[0], result);
        evtSetValue(event, argsPtr[1], 0);
    }
    INTPL_STEP(event) += 1;
    return 2;
}

s32 evt_sub_intpl_msec_get_value(EventEntry* event, BOOL isFirstCall) {
    s32 step = INTPL_STEP(event);
    s32 total = INTPL_TOTAL(event);
    f32 fStart, fEnd, result;

    if (step < total) {
        fStart = (f32)INTPL_START(event);
        fEnd = (f32)INTPL_END(event);
        result = intplGetValue(INTPL_MODE(event), step, total, fStart, fEnd);
    } else {
        fStart = (f32)INTPL_END(event);
        result = intplGetValue(INTPL_MODE(event), step, total, fStart, fStart);
    }

    event->lwData[0x9C / 4 - 0x9C / 4 + 0x9C / 4] = (s32)result;
    event->lwData[(0xA0 - 0x9C) / 4] = (step < total) ? 1 : 0;
    INTPL_STEP(event) += 1;
    return 2;
}

s32 evt_sub_intpl_msec_init(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    INTPL_MODE(event) = evtGetValue(event, args[0]);
    INTPL_START(event) = evtGetValue(event, args[1]);
    INTPL_END(event) = evtGetValue(event, args[2]);
    INTPL_STEP(event) = 0;
    INTPL_TOTAL(event) = evtGetValue(event, args[3]);
    event->lwData[(0x198 - 0x9C) / 4] = event->lwData[0];
    event->lwData[(0x19C - 0x9C) / 4] = event->lwData[1];
    return 2;
}

s32 evt_sub_intpl_get_float(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32* argsPtr = event->args;
    s32 arg0 = argsPtr[0];
    s32 arg1 = argsPtr[1];
    f32 fStart, fEnd, result;

    fStart = evtGetFloat(event, arg0);
    fEnd = evtGetFloat(event, arg1);
    result = intplGetValue(INTPL_MODE(event), INTPL_STEP(event), INTPL_TOTAL(event), fStart, fEnd);

    if (arg0 != (s32)0xEE74D80) {
        evtSetFloat(event, arg0, result);
    }
    if (arg1 != (s32)0xEE74D80) {
        if (INTPL_STEP(event) >= INTPL_TOTAL(event)) {
            evtSetValue(event, arg1, 0);
        } else {
            evtSetValue(event, arg1, 1);
        }
    }
    INTPL_STEP(event) += 1;
    return 2;
}

s32 evt_sub_intpl_init_float(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 scale = 1024.0f;
    s32 mode = evtGetValue(event, args[0]);
    f32 start = evtGetFloat(event, args[1]);
    f32 end = evtGetFloat(event, args[2]);
    s32 total = evtGetValue(event, args[3]);

    INTPL_MODE(event) = mode;
    INTPL_START(event) = (s32)(scale * start) - 0x0DB58580;
    INTPL_END(event) = (s32)(scale * end) - 0x0DB58580;
    INTPL_STEP(event) = 0;
    INTPL_TOTAL(event) = total;
    return 2;
}

s32 evt_sub_intpl_get_value_para(EventEntry* event, BOOL isFirstCall) {
    s32* argsPtr = event->args;
    f32 fStart = (f32)INTPL_START(event);
    f32 fEnd = (f32)INTPL_END(event);
    f32 result;

    result = intplGetValue(INTPL_MODE(event), INTPL_STEP(event), INTPL_TOTAL(event), fStart, fEnd);
    evtSetFloat(event, argsPtr[0], result);

    if (INTPL_STEP(event) >= INTPL_TOTAL(event)) {
        evtSetValue(event, argsPtr[1], 0);
    } else {
        evtSetValue(event, argsPtr[1], 1);
    }
    INTPL_STEP(event) += 1;
    return 2;
}

s32 evt_sub_intpl_get_value(EventEntry* event, BOOL isFirstCall) {
    s32 step = INTPL_STEP(event);
    s32 total = INTPL_TOTAL(event);
    f32 fStart = (f32)INTPL_START(event);
    f32 fEnd = (f32)INTPL_END(event);
    f32 result;

    result = intplGetValue(INTPL_MODE(event), step, total, fStart, fEnd);
    event->lwData[(0x9C - 0x9C) / 4] = (s32)result;
    event->lwData[(0xA0 - 0x9C) / 4] = (step < total) ? 1 : 0;
    INTPL_STEP(event) += 1;
    return 2;
}

s32 evt_sub_intpl_init(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    INTPL_MODE(event) = evtGetValue(event, args[0]);
    INTPL_START(event) = evtGetValue(event, args[1]);
    INTPL_END(event) = evtGetValue(event, args[2]);
    INTPL_STEP(event) = 0;
    INTPL_TOTAL(event) = evtGetValue(event, args[3]);
    return 2;
}

s32 evt_sub_check_intersect(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 x1 = evtGetFloat(event, args[0]);
    f32 y1 = evtGetFloat(event, args[1]);
    f32 z1 = evtGetFloat(event, args[2]);
    f32 sx1 = evtGetFloat(event, args[3]);
    f32 sy1 = evtGetFloat(event, args[4]);
    f32 sz1 = evtGetFloat(event, args[5]);
    f32 x2 = evtGetFloat(event, args[6]);
    f32 y2 = evtGetFloat(event, args[7]);
    f32 z2 = evtGetFloat(event, args[8]);
    f32 sx2 = evtGetFloat(event, args[9]);
    f32 sy2 = evtGetFloat(event, args[10]);
    f32 sz2 = evtGetFloat(event, args[11]);
    s32 result = 0;

    if (x1 <= sx1 && x1 >= sy1 && x1 >= sz1) {
        if (x2 >= sx1 || x2 <= sy1) {
            goto done;
        }
    }
    if (y1 <= sx1 && y1 >= sy1) {
        result = 1;
    }
done:
    evtSetValue(event, args[12], result);
    return 2;
}

s32 irai_mail_check(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    s32 arg0 = evtGetValue(event, args[0]);
    s32 result = 0;

    if (swGet(0x1505) != 0 && swGet(0x17) == 0) {
        u8 data[8];
        s32 i;
        for (i = 0; i < 8; i++) {
            data[i] = swByteGet(0x58D + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x17);
            pouchReceiveMail(0x25);
        }
    }

    if (swGet(0x150A) != 0 && swGet(0x18) == 0) {
        s32 i;
        for (i = 0; i < 8; i++) {
            swByteGet(0x595 + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x18);
            pouchReceiveMail(0x26);
        }
    }

    if (swGet(0x150B) != 0 && swGet(0x19) == 0) {
        s32 i;
        for (i = 0; i < 8; i++) {
            swByteGet(0x59D + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x19);
            pouchReceiveMail(0x27);
        }
    }

    if (swGet(0x150D) != 0 && swGet(0x1A) == 0) {
        s32 i;
        for (i = 0; i < 8; i++) {
            swByteGet(0x5A5 + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x1A);
            pouchReceiveMail(0x28);
        }
    }

    if (swGet(0x1512) != 0 && swGet(0x1B) == 0) {
        s32 i;
        for (i = 0; i < 8; i++) {
            swByteGet(0x5AD + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x1B);
            pouchReceiveMail(0x29);
        }
    }

    if (swGet(0x1518) != 0 && swGet(0x1C) == 0) {
        s32 i;
        for (i = 0; i < 8; i++) {
            swByteGet(0x5B5 + i);
        }
        if (arg0 != 0) {
            result = 1;
            swSet(0x1C);
            pouchReceiveMail(0x2A);
        }
    }

    evtSetValue(event, args[1], result);
    return 2;
}

s32 mail_evt_gor_04_keyoff_check(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    evtSetValue(event, args[0], marioCtrlOffChk() + marioKeyOffChk());
    return 2;
}
