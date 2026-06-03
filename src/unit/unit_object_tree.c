/* "unit_object_tree.c" - Tree enemy battle unit */

#include "battle/battle.h"
#include "battle/battle_event_cmd.h"
#include "evt/evt_cmd.h"

extern BattleWork* _battleWorkPointer;

extern s32 btlevtcmd_SetEventWait(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventAttack(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventDamage(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventConfusion(EventEntry*, BOOL);
extern s32 btlevtcmd_StartWaitEvent(EventEntry*, BOOL);
extern s32 btldefaultevt_Confuse(EventEntry*, BOOL);

USER_FUNC(_rumble_tree);
USER_FUNC(_turn_tree);

//.sdata2
static char str_8041E780[] = "M_S_1";
static char str_8041E788[] = "M_O_1";
static const f32 float_0_8041e790 = 0.0f;

//.rodata
static char str_802F9E58[] = "btl_un_tree";
static char str_802F9E64[] = "SFX_BTL_DAMAGE1";
static char str_802F9E74[] = "SFX_BTL_DAMAGE_FIRE1";
static char str_802F9E8C[] = "SFX_BTL_DAMAGE_ICE1";
static char str_802F9EA0[] = "SFX_BTL_DAMAGE_BIRIBIRI1";
static char str_802F9EBC[] = "b_mario";

//.sdata
static s32 defence[2] = { 0, 0 };
static s32 defence_attr[2] = { 0, 0 };
static s32 regist[6] = { 0, 0, 0, 0, 0, 0 };
static s32 pose_table[] = {
    0x0000001C, (s32)str_8041E780,
    0x0000001F, (s32)str_8041E780,
    0x00000027, (s32)str_8041E788,
    0x00000045, (s32)str_8041E780,
};
static s32 data_table[2] = { 0, 0 };
static s32 attack_event[] = {
    EVT_CMD(2, 0x5B), (s32)btlevtcmd_StartWaitEvent, (s32)-2,
    EVT_RETURN_DONE, END
};
static s32 wait_event[] = {
    EVT_RETURN_DONE, END
};
static s32 damage_event[] = {
    EVT_CMD(2, 0x32), 0xFE363C8A, (s32)-2,
    EVT_CMD(2, 0x32), 0xFE363C8B, 0x00000001,
    EVT_CMD(2, 0x5B), (s32)_rumble_tree, (s32)-2,
    EVT_CMD(2, 0x5B), (s32)_turn_tree, (s32)-2,
    EVT_RETURN_DONE, END
};

//.data
static s32 parts[] = {
    0x00000001, (s32)str_802F9E58, (s32)str_802F9EBC,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x41C80000, 0x00000000, 0x00000000,
    0x41F00000, 0x00000000,
    0x0014001E, 0x00FF0000,
    (s32)defence, (s32)defence_attr,
    0x03000009, 0x00000000, (s32)pose_table,
};

static s32 init_event[] = {
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventWait, (s32)-2, (s32)wait_event,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventAttack, (s32)-2, (s32)attack_event,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventDamage, (s32)-2, (s32)damage_event,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventConfusion, (s32)-2, (s32)btldefaultevt_Confuse,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetMaxMoveCount, (s32)-2, 0x00000000,
    EVT_CMD(2, 0x5B), (s32)btlevtcmd_StartWaitEvent, (s32)-2,
    EVT_RETURN_DONE, END
};

static s32 _data[] = {
    0xFFFFFFFB, 0x00000000, 0x0000000F, 0x00000000,
    0xFFFFFFF1, 0x00000000, 0x0000000E, 0x00000000,
    0xFFFFFFF3, 0x00000000, 0x0000000C, 0x00000000,
    0xFFFFFFF5, 0x00000000, 0x0000000A, 0x00000000,
    0xFFFFFFF7, 0x00000000, 0x00000008, 0x00000000,
    0xFFFFFFF9, 0x00000000, 0x00000006, 0x00000000,
    0xFFFFFFFA, 0x00000000, 0x00000005, 0x00000000,
    0xFFFFFFFC, 0x00000000, 0x00000002, 0x00000000,
    0xFFFFFFFE, 0x00000000, 0x00000000,
};

static s32 unit_object_tree[] = {
    0x000000D8, (s32)str_802F9E58, 0x03E70000, 0x01010200,
    0x00000064, 0x00090018, 0x00180000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000,
    0x41400000, 0x00000000, 0x00000000,
    0x41400000, 0x4179999A,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x0000FF00, 0x64140000,
    0x00000000, 0x00000000, 0x00000000,
    (s32)str_802F9E64, (s32)str_802F9E74,
    (s32)str_802F9E8C, (s32)str_802F9EA0,
    0x12003001, (s32)regist, 0x01000000,
    (s32)parts, (s32)init_event, (s32)data_table,
};

/* 801A36EC 001A06EC size 0xE8 - first in source, second in binary */
USER_FUNC(_rumble_tree) {
    s32* args = event->args;
    s32 unitId = evtGetValue(event, args[0]);
    s32 transId = BattleTransID(event, unitId);
    BattleWorkUnit* unit = BattleGetUnitPtr(_battleWorkPointer, transId);
    BattleStageObject* obj;
    if (!unit) {
        return EVT_RETURN_DONE;
    }
    obj = BattleGetObjectPtr(*(s32*)((s8*)unit + 0xB2C));
    if (isFirstCall) {
        event->userdata[0] = 0;
    }
    {
        s32 idx = event->userdata[0];
        f32 delta = (f32)_data[idx];
        obj->mPosition.y += delta;
        event->userdata[0]++;
    }
    if (event->userdata[0] >= 0x22) {
        return EVT_RETURN_DONE;
    }
    return 0;
}

/* 801A3638 001A0638 size 0xB4 - second in source, first in binary */
USER_FUNC(_turn_tree) {
    s32 transId = BattleTransID(event, -2);
    BattleWorkUnit* unit = BattleGetUnitPtr(_battleWorkPointer, transId);
    BattleStageObject* obj = BattleGetObjectPtr(*(s32*)((s8*)unit + 0xB2C));
    if (isFirstCall) {
        event->userdata[0] = 0;
    }
    event->userdata[0] += 3;
    if (event->userdata[0] > 0x5A) {
        obj->mShakeAngle = 0.0f;
        return EVT_RETURN_DONE;
    }
    obj->mShakeAngle = (f32)event->userdata[0];
    return 0;
}
