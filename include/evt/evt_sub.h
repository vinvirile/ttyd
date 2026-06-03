#pragma once

#include <dolphin/types.h>

struct EventEntry;
s32 evt_sub_intpl_init(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_get_value(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_get_value_para(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_init_float(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_get_float(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_msec_init(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_msec_get_value(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_intpl_msec_get_value_para(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_spline_init(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_spline_get_value(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_spline_get_value_manual(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_spline_free(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_sincos(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_rumble_onoff(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_random(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_stopwatch(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_area_check(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_dist(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_dir(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_system_flag(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_system_flag_onoff(struct EventEntry* event, BOOL isFirstCall);
s32 evt_key_get_dir(struct EventEntry* event, BOOL isFirstCall);
s32 evt_key_get_button(struct EventEntry* event, BOOL isFirstCall);
s32 evt_key_get_buttontrg(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_mapname(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_areaname(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_status_gauge_force_open(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_status_gauge_force_close(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_status_gauge_check_update(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_save_playtime(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_load_progresstime(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_language(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_animgroup_async(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_countdown_start(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_countdown_get_status(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_countdown_end(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_countdown_flag_onoff(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_countdown_set_restart_time(struct EventEntry* event, BOOL isFirstCall);
s32 coingetDisp(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_get_coin(struct EventEntry* event, BOOL isFirstCall);
s32 evt_sub_check_intersect(struct EventEntry* event, BOOL isFirstCall);
s32 irai_mail_check(struct EventEntry* event, BOOL isFirstCall);
s32 mail_evt_gor_04_keyoff_check(struct EventEntry* event, BOOL isFirstCall);
