/* "unit_object_switch.c" - Switch enemy battle unit */

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
extern s32 evt_map_playanim(EventEntry*, BOOL);

USER_FUNC(_rumble_switch);

//.sdata2
static char str_8041E798[] = "M_S_1";
static char str_8041E7A0[] = "M_O_1";

//.rodata
static char str_802F9ED0[] = "btl_un_switch";
static char str_802F9EE0[] = "SFX_BTL_DAMAGE1";
static char str_802F9EF0[] = "SFX_BTL_DAMAGE_FIRE1";
static char str_802F9F08[] = "SFX_BTL_DAMAGE_ICE1";
static char str_802F9F1C[] = "SFX_BTL_DAMAGE_BIRIBIRI1";
static char str_802F9F38[] = "b_mario";
static char str_802F9F40[] = "switch_a";

//.sdata
static s32 defence[2] = { 0, 0 };
static s32 defence_attr[2] = { 0, 0 };
static s32 regist[6] = { 0, 0, 0, 0, 0, 0 };
static s32 pose_table[] = {
    0x0000001C, (s32)str_8041E798,
    0x0000001F, (s32)str_8041E798,
    0x00000027, (s32)str_8041E7A0,
    0x00000045, (s32)str_8041E798,
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
    EVT_CMD(4, 0x5B), (s32)btlevtcmd_CheckDamageCode, (s32)-2,
    0x00000100, 0xFE363C80,
    EVT_CMD(2, 0x18), 0xFE363C80, 0x00000000,
    EVT_CMD(2, 0x5B), (s32)_rumble_switch, (s32)-2, 0x00000021,
    EVT_CMD(4, 0x5B), (s32)evt_map_playanim, (s32)str_802F9F40,
    0x00000000, 0x00000000,
    EVT_CMD(1, 0x0A), 0x000001F4,
    EVT_RETURN_DONE, END
};

//.data
static s32 parts[] = {
    0x00000001, (s32)str_802F9ED0, (s32)str_802F9F38,
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

static s32 unit_object_switch[] = {
    0x000000D9, (s32)str_802F9ED0, 0x03E70000, 0x01010200,
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
    (s32)str_802F9EE0, (s32)str_802F9EF0,
    (s32)str_802F9F08, (s32)str_802F9F1C,
    0x12003001, (s32)regist, 0x01000000,
    (s32)parts, (s32)init_event, (s32)data_table,
};

/* 801A37D4 001A07D4 size 0xE8 */
USER_FUNC(_rumble_switch) {
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
