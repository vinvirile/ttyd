/* "unit_hyper_kuriboo.c" - Hyper Kuriboo battle unit (REL gra) */

#include "evt/evt_cmd.h"
#include "manager/evtmgr_cmd.h"
#include "battle/battle.h"
#include "battle/battle_event_cmd.h"
#include "system.h"

extern s32 weaponGetPowerDefault(s32, s32*);
extern s32 btldefaultevt_Damage(EventEntry*, BOOL);
extern s32 btldefaultevt_Dummy(EventEntry*, BOOL);
extern f32 angleABf(f32, f32, f32, f32);
extern s32 btlevtcmd_SetEventWait(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventAttack(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventDamage(EventEntry*, BOOL);
extern s32 btlevtcmd_SetEventConfusion(EventEntry*, BOOL);
extern s32 btlevtcmd_SetRunSound(EventEntry*, BOOL);
extern s32 btlevtcmd_SetWalkSound(EventEntry*, BOOL);
extern s32 btlevtcmd_SetJumpSound(EventEntry*, BOOL);
extern s32 btlevtcmd_StartWaitEvent(EventEntry*, BOOL);
extern s32 btlevtcmd_SetUnitWork(EventEntry*, BOOL);
extern s32 btlevtcmd_AnimeChangePoseFromTable(EventEntry*, BOOL);

USER_FUNC(krb_get_dir);

//.rodata
static char str_00001030[] = "btl_un_hyper_kuriboo";
static char str_00001048[] = "SFX_ENM_KURI_DAMAGED1";
static char str_00001060[] = "SFX_BTL_DAMAGE_FIRE1";
static char str_00001078[] = "SFX_BTL_DAMAGE_ICE1";
static char str_0000108C[] = "SFX_BTL_DAMAGE_BIRIBIRI1";
static char str_000010A8[] = "KUR_N_1";
static char str_000010B0[] = "KUR_Y_1";
static char str_000010B8[] = "KUR_K_1";
static char str_000010C0[] = "KUR_I_1";
static char str_000010C8[] = "KUR_S_1";
static char str_000010D0[] = "KUR_Q_1";
static char str_000010D8[] = "KUR_D_1";
static char str_000010E0[] = "KUR_A_1";
static char str_000010E8[] = "KUR_A_2";
static char str_000010F0[] = "KUR_R_1";
static char str_000010F8[] = "KUR_R_2";
static char str_00001100[] = "KUR_W_1";
static char str_00001108[] = "KUR_T_1";
static char str_00001110[] = "c_kuribo_h";
static char str_0000111C[] = "SFX_ENM_KURI_MOVE1";
static char str_00001130[] = "SFX_ENM_KURI_MOVE2";
static char str_00001144[] = "SFX_ENM_KURI_JUMP1";
static char str_00001158[] = "SFX_ENM_KURI_LANDING1";
static char str_00001170[] = "SFX_ENM_CHARGE1";
static char str_00001184[] = "crystal_n64";
static char str_00001190[] = "SFX_ENM_KURI_ATTACK1";

static const f32 zero_gra = 0.0f;

//.data
static f32 negone_one[2] = { -1.0f, 1.0f };
static f32 neg_one = -1.0f;
static const f32 __local_PI = 3.1415927f;
static const f32 __local_3PIO2 = 4.712389f;
static f32 gap_04_data = 0.0f;
static f32 one_negone[2] = { 1.0f, -1.0f };
static const f32 __local_PIO2 = 1.5707964f;
static f32 one_f = 1.0f;

static f32 negone_one_2[2] = { -1.0f, 1.0f };
static f32 neg_one_2 = -1.0f;
static const f32 __local_PI_2 = 3.1415927f;
static const f32 __local_3PIO2_2 = 4.712389f;
static f32 gap_04_data_2 = 0.0f;
static f32 one_negone_2[2] = { 1.0f, -1.0f };
static const f32 __local_PIO2_2 = 1.5707964f;
static f32 one_f_2 = 1.0f;

static s32 unit_hyper_kuriboo[] = {
    0x00000042, (s32)str_00001030,
    0x00080000, 0x01010F00,
    0x02460146, 0x270F0017,
    0x00190005, 0x00190000,
    0x00000000, 0x41400000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000,
    0x41400000, 0x00000000, 0xC1200000,
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x41380000,
    0x00000000, 0x00000000, 0x41400000,
    0x41900000, 0x00000000, 0x00000000,
    0x41400000, 0x00000000, 0x41B80000,
    0x41C80000, 0x00000002, 0x64140000,
    0x00000000, 0x00000000, 0x00000000,
    (s32)str_00001048, (s32)str_00001060,
    (s32)str_00001078, (s32)str_0000108C,
    0x00040000, 0x00000000, 0x01000000,
    0x00000000, 0x00000000, 0x00000000,
};

static s32 pose_table[] = {
    0x00000001, (s32)str_000010A8,
    0x00000002, (s32)str_000010B0,
    0x00000009, (s32)str_000010B0,
    0x00000005, (s32)str_000010B8,
    0x00000004, (s32)str_000010C0,
    0x00000003, (s32)str_000010C0,
    0x0000001C, (s32)str_000010C8,
    0x0000001D, (s32)str_000010D0,
    0x0000001E, (s32)str_000010D0,
    0x0000001F, (s32)str_000010C8,
    0x00000027, (s32)str_000010D8,
    0x00000032, (s32)str_000010E0,
    0x00000033, (s32)str_000010E8,
    0x0000002A, (s32)str_000010F0,
    0x00000029, (s32)str_000010F8,
    0x00000028, (s32)str_00001100,
    0x00000038, (s32)str_000010C0,
    0x00000039, (s32)str_000010C0,
    0x00000041, (s32)str_00001108,
    0x00000045, (s32)str_000010C8,
};

static s32 data_table[] = {
    0x00000030, (s32)btldefaultevt_Dummy,
    0x00000000, 0x00000000,
};

static s32 defence[2] = { 0, 0 };
static s32 defence_attr[2] = { 0, 0 };
static s32 regist[6] = {
    0x505A5064, 0x50646450,
    0x645F645F, 0x645F505A,
    0x50645064, 0x645F0000,
};

static s32 parts[] = {
    0x00000001, (s32)str_00001030, (s32)str_00001110,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x41B00000, 0x00000000, 0x00000000, 0x41F00000,
    0x00000000, 0x0014001E, 0x00FF0000,
    (s32)defence, (s32)defence_attr,
    0x00000009, 0x00000000, (s32)pose_table,
};

static s32 weapon_kuriboo_attack[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x64000001, 0x3F800000, 0x01010101, (s32)weaponGetPowerDefault,
    0x00000002, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01101260, 0x20001000, 0x00000302, 0x00000000,
    0x00005087, 0x00000000, 0x80002004, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000064, 0x00000000, 0x00000000,
};

static s32 weapon_kuriboo_charge[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x64000000, 0x3F800000, 0x01010101, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01004020, 0x20000000, 0x00000302, 0x00000000,
    0x00000108, 0x000007FF, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000006, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000064, 0x00000000, 0x00000000,
};

// attack_event (0x9F8 bytes) - TODO: large event script, deferred

static s32 init_event[] = {
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventWait, (s32)-2, 0x00000000,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventAttack, (s32)-2, 0x00000000,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventDamage, (s32)-2, 0x00000000,
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_SetEventConfusion, (s32)-2, 0x00000000,
    EVT_CMD(7, 0x5B), (s32)btlevtcmd_SetRunSound, (s32)-2,
    (s32)str_0000111C, (s32)str_00001130,
    0x00000000, 0x00000006, 0x00000006,
    EVT_CMD(7, 0x5B), (s32)btlevtcmd_SetWalkSound, (s32)-2,
    (s32)str_0000111C, (s32)str_00001130,
    0x00000000, 0x00000006, 0x00000006,
    EVT_CMD(4, 0x5B), (s32)btlevtcmd_SetJumpSound, (s32)-2,
    (s32)str_00001144, (s32)str_00001158,
    EVT_CMD(4, 0x5B), (s32)btlevtcmd_SetUnitWork, (s32)-2,
    0x00000000, 0x00000000,
    EVT_CMD(2, 0x5B), (s32)btlevtcmd_StartWaitEvent, (s32)-2,
    0x00000002, 0x00000001,
    EVT_RETURN_DONE, END
};

static s32 damage_event[] = {
    EVT_CMD(2, 0x32), 0xFE363C8A, (s32)-2,
    EVT_CMD(2, 0x32), 0xFE363C8B, 0x00000001,
    EVT_CMD(1, 0x5E), (s32)btldefaultevt_Damage,
    0x00000002, 0x00000001,
    EVT_RETURN_DONE, END
};

static s32 phase_event[] = {
    0x00000002, 0x00000001,
};

static s32 wait_event[] = {
    EVT_CMD(3, 0x5B), (s32)btlevtcmd_AnimeChangePoseFromTable, (s32)-2,
    0x00000001, 0x00000002, 0x00000001,
    EVT_RETURN_DONE, END
};

s32 krb_get_dir(EventEntry* event, BOOL isFirstCall) {
    s32* args = event->args;
    f32 dx = evtGetFloat(event, args[0]);
    f32 dy = evtGetFloat(event, args[1]);
    f32 px = evtGetFloat(event, args[2]);
    f32 py = evtGetFloat(event, args[3]);
    s32 out = evtGetValue(event, args[4]);

    if (px - dx == 0.0f && py - dy == 0.0f) {
        evtSetValue(event, args[4], out);
        return 2;
    }

    evtSetValue(event, args[4], (s32)angleABf(px - dx, py - dy, 0.0f, 0.0f));
    return 2;
}
