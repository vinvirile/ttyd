/* "battle_weapon_power.c" - Weapon power calculation functions */

#include "battle/battle_weapon_power.h"
#include "battle/battle.h"
#include "battle/battle_unit.h"
#include "mario/mario_pouch.h"

extern BattleWork* _battleWorkPointer;

s32 weaponGetPowerDefault(s32 unused, s32* table) {
    return *(s32*)((char*)table + 0x20);
}

s32 weaponGetFPPowerDefault(s32 unused, s32* table) {
    return *(s32*)((char*)table + 0x44);
}

#pragma dont_inline on
s32 weaponGetPowerFromMarioJumpLv(BattleWorkUnit* unit, s32* table) {
    s32 i = *(s32*)((char*)_battleWorkPointer + 0x1CB8);
    s32 isGullible = (i >> 30) & 1;
    s32 level = pouchGetPtr()->jumpLevel;
    s32 idx = (level - 1) * 2 + isGullible;
    s32 extra;
    if (idx >= 6) idx = 5;
    extra = *(u8*)((char*)unit + 0x2FB);
    i = *(s32*)((char*)table + idx * 4 + 0x20);
    if (extra != 0) i += extra;
    return i;
}

s32 weaponGetPowerFromMarioHammerLv(BattleWorkUnit* unit, s32* table) {
    s32 i = *(s32*)((char*)_battleWorkPointer + 0x1CB8);
    s32 isGullible = (i >> 30) & 1;
    s32 level = pouchGetPtr()->hammerLevel;
    s32 idx = (level - 1) * 2 + isGullible;
    s32 extra;
    if (idx >= 6) idx = 5;
    extra = *(u8*)((char*)unit + 0x2FC);
    i = *(s32*)((char*)table + idx * 4 + 0x20);
    if (extra != 0) i += extra;
    return i;
}
#pragma dont_inline off

s32 weaponGetPowerOverlapJump1(BattleWorkUnit* unit, s32* table) {
    s32 mult = *(s32*)((char*)table + 0x3C);
    s32 badge = pouchEquipCheckBadge(*(s32*)((char*)table + 0x38));
    return weaponGetPowerFromMarioJumpLv(unit, table) + mult * badge;
}

s32 weaponGetPowerOverlapHammer1(BattleWorkUnit* unit, s32* table) {
    s32 mult = *(s32*)((char*)table + 0x3C);
    s32 badge = pouchEquipCheckBadge(*(s32*)((char*)table + 0x38));
    return weaponGetPowerFromMarioHammerLv(unit, table) + mult * badge;
}

s32 weaponGetPowerFromPartyAttackLv(BattleWorkUnit* unit, s32* table) {
    s32 i;
    s32 level;
    s32 idx;
    i = BattleTransPartyId(unit->currentKind);
    level = pouchGetPartyAttackLv(i);
    i = *(s32*)((char*)_battleWorkPointer + 0x1CB8);
    i = (i >> 30) & 1;
    idx = i + level * 2;
    if (idx >= 6) idx = 5;
    return *(s32*)((char*)table + idx * 4 + 0x20);
}

s32 weaponGetPowerFromGulliblePartyAttackLv(BattleWorkUnit* unit, s32* table) {
    s32 level;
    level = BattleTransPartyId(unit->currentKind);
    level = pouchGetPartyAttackLv(level);
    if (level >= 6) level = 5;
    return *(s32*)((char*)table + level * 4 + 0x20);
}

s32 weaponGetACOutputParam(s32 unused, s32* table) {
    s32 i = *(s32*)((char*)table + 0x20);
    s32 base = *(s32*)((char*)_battleWorkPointer + i * 4 + 0x1CE8);
    s32 extra = *(s32*)((char*)table + 0x24);
    base += extra;
    if (base < 0) base = 0;
    return base;
}

s32 weaponGetPowerHPHalf(s32 unused, s32 unused2, s32* table) {
    s32 val;
    s32 result;
    val = *(s16*)((char*)table + 0x10C);
    result = val / 2;
    if (val & 1) result++;
    return result;
}

s32 weaponGetPowerHPHalf2(s32 unused, s32 unused2, s32* table) {
    return *(s16*)((char*)table + 0x10C) / 2;
}

s32 weaponGetFPPowerFPHalf(s32 unused, s32 unused2, s32* table) {
    s32 val;
    s32 result;
    val = *(s16*)((char*)table + 0x112);
    result = val / 2;
    if (val & 1) result++;
    return result;
}

s32 weaponGetPowerTatsumaki(s32 unused, s32* table) {
    s32 base;
    s32 badge;
    s32 mult;
    base = *(s32*)((char*)table + 0x20);
    BattleGetMarioPtr(_battleWorkPointer);
    badge = pouchEquipCheckBadge(0xF3);
    mult = *(s32*)((char*)table + 0x24);
    return base + mult * badge;
}
