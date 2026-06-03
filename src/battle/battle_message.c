/* "battle_message.c" - Battle message display */

#include "battle/battle.h"
#include "data/item_data.h"
#include "driver/dispdrv.h"
#include "driver/msgdrv.h"
#include "driver/windowdrv.h"
#include "manager/evtmgr.h"
#include "manager/fontmgr.h"

extern BattleWork* _battleWorkPointer;
extern ItemData itemDataTable[];
extern u32 FontGetMessageWidthLine(char* msg, u16* xPosOut);
extern void FontDrawMessage(s32 xPos, s32 yPos, char* msg);

void _disp(CameraId camId, void* param);

//.rodata
static const f64 double_to_int_802fa148 = 4503601774854144.0;

//.sdata2
static const u32 dat_8041e840 = 0xFFFFFFFF;
static const f32 float_10_8041e844 = 10.0f;
static const f32 float_neg29_8041e848 = -29.0f;
static const f32 float_3_8041e84c = 3.0f;
static const f32 float_900_8041e850 = 900.0f;

USER_FUNC(btlevtcmd_AnnounceMessage) {
    BattleWork* work;
    s32* args;
    s32 msgType, xPos, yPos;
    s32 displayFrames;
    char* msgStr;
    s32 itemId;
    char* msg;

    work = _battleWorkPointer;
    args = event->args;
    msgType = evtGetValue(event, args[0]);
    xPos = evtGetValue(event, args[1]);
    yPos = evtGetValue(event, args[2]);
    msgStr = (char*)evtGetValue(event, args[3]);
    displayFrames = evtGetValue(event, args[4]);

    if (msgStr == NULL) {
        return EVT_RETURN_DONE;
    }

    if (isFirstCall) {
        event->userdata[0] = displayFrames;
        work->field_0x18C70 = (s16)xPos;
        work->field_0x18C82 = (s16)yPos;
        msg = (char*)msgSearch(msgStr);
        if (msgType == 1) {
            itemId = *(s32*)((char*)work + 0x18C74);
            sprintf((char*)work + 0x18C84, msg,
                    itemDataTable[itemId].id);
        } else {
            sprintf((char*)work + 0x18C84, msg);
        }
    }

    dispEntry(8, 1, _disp, NULL, 900.0f);

    event->userdata[0]--;
    return event->userdata[0] == 0;
}

void _disp(CameraId camId, void* param) {
    BattleWork* work;
    s32 xPos, yPos;
    u32 msgInfo;
    u16 width, lines, xPosU;
    f32 fXPos, fYPos;
    s32 unk;
    GXColor color;

    work = _battleWorkPointer;
    xPos = work->field_0x18C70;
    yPos = work->field_0x18C82;

    fXPos = (f32)xPos;
    fYPos = (f32)(yPos + 0x78);

    msgInfo = FontGetMessageWidthLine((char*)work + 0x18C84, &xPosU);
    width = (u16)(msgInfo & 0xFFFF);
    lines = (u16)((msgInfo >> 16) & 0x7FFF);

    xPosU++;
    unk = (s32)xPosU * 0x1D + 3;

    fXPos -= (f32)lines;

    *(u32*)&color = dat_8041e840;

    windowDispGX_Waku_col(0, color,
        fXPos - 10.0f,
        fYPos + 3.0f - 29.0f,
        (f32)(width + 20), (f32)unk, 10.0f);

    FontDrawStart();
    FontDrawMessage((s32)fXPos, (s32)fYPos - 29, (char*)work + 0x18C84);
}

USER_FUNC(btlevtcmd_AnnounceSetParam) {
    s32* args;
    s32 paramIndex, paramValue;

    args = event->args;
    paramIndex = evtGetValue(event, args[0]);
    paramValue = evtGetValue(event, args[1]);
    *(s32*)((char*)_battleWorkPointer + 0x18C74 + paramIndex * 4) = paramValue;
    return EVT_RETURN_DONE;
}
