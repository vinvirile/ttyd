#pragma once

#include "manager/evtmgr.h"

s32 evt_pouch_get_coin(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_add_coin(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_set_coin(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_supercoin(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_set_supercoin(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_starpiece(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_add_starpiece(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_starstone(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_add_item(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_remove_item(EventEntry* event, BOOL isFirstCall);
s32 N_evt_pouch_remove_item_index(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_add_keepitem(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_remove_keepitem(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_check_item(EventEntry* event, BOOL isFirstCall);
s32 L_evt_pouch_get_hp(EventEntry* event, BOOL isFirstCall);
s32 L_evt_pouch_set_hp(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_max_ap(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_set_ap(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_party_join(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_party_left(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_mario_recovery(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_all_party_recovery(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_haveitemcnt(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_havebadgecnt(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_equipbadgecnt(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_majinai_set(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_receive_mail(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_open_mail(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_set_yoshiname(EventEntry* event, BOOL isFirstCall);
s32 evt_pouch_get_yoshiname(EventEntry* event, BOOL isFirstCall);
