#include "evt/evt_cmd.h"
#include "mario/mario_pouch.h"
#include "battle/battle.h"
#include "battle/battle_event_cmd.h"

extern s32 weaponGetPowerDefault(s32, s32*);
extern s32 btldefaultevt_Confuse(EventEntry*, BOOL);
extern s32 btldefaultevt_Damage(EventEntry*, BOOL);
extern s32 subsetevt_confuse_flustered(EventEntry*, BOOL);
extern s32 evt_sub_random(EventEntry*, BOOL);

USER_FUNC(marioAttackEvent_NormalJump);
USER_FUNC(marioAttackEvent_KururinJump);
USER_FUNC(marioAttackEvent_NormalHammer);
USER_FUNC(marioAttackEvent_KaitenHammer);
USER_FUNC(jump_attack_voice);
USER_FUNC(wait_event_fmario);
USER_FUNC(damage_event_fmario);
USER_FUNC(attack_event_fmario);
USER_FUNC(init_event_fmario);
USER_FUNC(dead_event_fmario);
USER_FUNC(evt_btl_camera_set_mode);
USER_FUNC(evt_btl_camera_set_moveto);
USER_FUNC(evt_btl_camera_set_zoom);
USER_FUNC(evt_btl_camera_set_moveSpeedLv);
USER_FUNC(evt_btl_camera_set_homing_unitparts);

//.rodata
static char str_802F0E50[] = "btl_un_hatena";
static char str_802F0E60[] = "SFX_MARIO_DAMAGE1";
static char str_802F0E74[] = "SFX_BTL_DAMAGE_FIRE1";
static char str_802F0E8C[] = "SFX_BTL_DAMAGE_ICE1";
static char str_802F0EA0[] = "SFX_BTL_DAMAGE_BIRIBIRI1";
static char str_802F0EBC[] = "EM_N_1";
static char str_802F0EC4[] = "EM_Y_2";
static char str_802F0ECC[] = "EM_K_1";
static char str_802F0ED4[] = "EM_S_1";
static char str_802F0EDC[] = "EM_D_1";
static char str_802F0EE4[] = "EM_A_1";
static char str_802F0EEC[] = "EM_R_1";
static char str_802F0EF4[] = "EM_W_1";
static char str_802F0EFC[] = "EM_D_3";
static char str_802F0F04[] = "EM_T_1";
static char str_802F0F0C[] = "c_mario";
static char str_802F0F14[] = "SFX_BOSS_NM_MOVE1L";
static char str_802F0F28[] = "SFX_BOSS_NM_MOVE1R";
static char str_802F0F3C[] = "SFX_BOSS_VOICE_NM_AC1_1";
static char str_802F0F54[] = "SFX_BOSS_VOICE_NM_AC1_2";
static char str_802F0F6C[] = "SFX_BOSS_VOICE_NM_AC1_5";
static char str_802F0F84[] = "EM_J_1B";
static char str_802F0F8C[] = "SFX_BOSS_NM_JUMP1";
static char str_802F0FA0[] = "SFX_BOSS_NM_DOWN1";
static char str_802F0FB4[] = "EM_D_2";
static char str_802F0FBC[] = "EM_Z_2";
static char str_802F0FC4[] = "SFX_BOSS_NM_JUMP_KURURIN1";
static char str_802F0FE0[] = "EM_A_1A";
static char str_802F0FE8[] = "SFX_BOSS_NM_HAMMER_FURU2";
static char str_802F1004[] = "EM_A_1B";
static char str_802F100C[] = "EM_A_1C";
static char str_802F1014[] = "SFX_BOSS_NM_HAMMER_DON2";

//.data
static f32 negone_one[2] = { -1.0f, 1.0f };
static f32 neg_one = -1.0f;
static const f32 __local_PI = 3.1415927f;
static const f32 __local_3PIO2 = 4.712389f;
static f32 gap_04_802F7C6C_data = 0.0f;
static f32 one_negone[2] = { 1.0f, -1.0f };
static const f32 __local_PIO2 = 1.5707964f;
static f32 one_f = 1.0f;

static f32 negone_one_2[2] = { -1.0f, 1.0f };
static f32 neg_one_2 = -1.0f;
static const f32 __local_PI_2 = 3.1415927f;
static const f32 __local_3PIO2_2 = 4.712389f;
static f32 gap_04_802F7C94_data = 0.0f;
static f32 one_negone_2[2] = { 1.0f, -1.0f };
static const f32 __local_PIO2_2 = 1.5707964f;
static f32 one_f_2 = 1.0f;

static s32 defence_fmario[2] = { 0, 0 };
static s32 defence_attr_fmario[2] = { 0x02020202, 0x02000000 };
static s32 regist_fmario[6] = { 0x64, 0, 0, 0, 0, 0 };

static s32 pose_table_fmario_stay[] = {
    0x00000001, (s32)str_802F0EBC,
    0x00000002, (s32)str_802F0EC4,
    0x00000009, (s32)str_802F0EC4,
    0x00000005, (s32)str_802F0ECC,
    0x00000004, (s32)str_802F0ECC,
    0x00000003, (s32)str_802F0ECC,
    0x0000001C, (s32)str_802F0ED4,
    0x00000027, (s32)str_802F0EDC,
    0x00000032, (s32)str_802F0EE4,
    0x0000002A, (s32)str_802F0EEC,
    0x00000028, (s32)str_802F0EF4,
    0x00000038, (s32)str_802F0EFC,
    0x00000039, (s32)str_802F0EFC,
    0x00000041, (s32)str_802F0F04,
    0x00000045, (s32)str_802F0ED4,
};

static s32 data_table_fmario[] = {
    0x00000031, (s32)dead_event_fmario,
    0x00000000, 0x00000000,
};

static s32 parts_fmario[] = {
    0x00000001, (s32)str_802F0E50, (s32)str_802F0F0C,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x40400000, 0x41D80000, 0x00000000, 0x00000000,
    0x41F00000, 0x00000000, 0x0014001E, 0x00FF0000,
    (s32)defence_fmario, (s32)defence_attr_fmario,
    0x00000009, 0x00000000, (s32)pose_table_fmario_stay,
};

s32 vivian_comeback(void) {
    PouchData* pouch = pouchGetPtr();
    pouch->partyData[7].flags &= 0xFFFFFF80;
    return 1;
}
