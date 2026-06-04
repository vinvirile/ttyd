#include "evt/evt_hit.h"
#include "driver/hitdrv.h"

extern s32 hitDmgretPtr;
extern void hitBindUpdate(const char* name);
extern void hitObjFlagOn(const char* name, s32 flags);
extern void hitObjFlagOff(const char* name, s32 flags);
extern void hitGrpFlagOn(const char* name, s32 flags);
extern void hitGrpFlagOff(const char* name, s32 flags);
extern void hitObjAttrOn(s32 attr, s32 flags);
extern void hitObjAttrOff(s32 attr, s32 flags);

// Binary order (reversed in source for -inline deferred):
// evt_hitobj_name_callback (1st), name_callback_sub (2nd), ..., evt_hitobj_onoff (last)
// Source must have: evt_hitobj_onoff (first), ..., evt_hitobj_name_callback (last)

USER_FUNC(evt_hitobj_onoff) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	s32 val2 = evtGetValue(event, args[2]);
	if (val1 != 0) {
		hitObjFlagOn((const char*)val0, 1);
	} else if (val2 != 0) {
		hitObjFlagOff((const char*)val0, 1);
	} else {
		hitObjFlagOn((const char*)val0, 1);
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_hit_damage_return_set) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	hitDmgretPtr = (s32)val0;
	// TODO: complete function
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_hit_bind_mapobj) {
	s32* args = event->args;
	const char* name = (const char*)evtGetValue(event, args[0]);
	const char* mapObjName = (const char*)evtGetValue(event, args[1]);
	hitBindMapObj(name, mapObjName);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_hit_bind_update) {
	const char* name = (const char*)evtGetValue(event, event->args[0]);
	hitBindUpdate(name);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_hit_get_position) {
	s32* args = event->args;
	Vec pos;
	const char* name = (const char*)evtGetValue(event, args[0]);
	hitObjGetPos(name, &pos);
	evtSetFloat(event, args[1], pos.x);
	evtSetFloat(event, args[2], pos.y);
	evtSetFloat(event, args[3], pos.z);
	return EVT_RETURN_DONE;
}

USER_FUNC(L_evt_hitobj_flag_onoff) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	s32 val2 = evtGetValue(event, args[2]);
	s32 val3 = evtGetValue(event, args[3]);
	if (val0 != 0) {
		if (val1 != 0) {
			hitGrpFlagOn((const char*)val2, val3);
		} else {
			hitGrpFlagOff((const char*)val2, val3);
		}
	} else {
		if (val1 != 0) {
			hitObjFlagOn((const char*)val2, val3);
		} else {
			hitObjFlagOff((const char*)val2, val3);
		}
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_hitobj_attr_onoff) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	s32 val2 = evtGetValue(event, args[2]);
	s32 val3 = evtGetValue(event, args[3]);
	if (val0 != 0) {
		hitObjAttrOn(val2, val3);
	} else if (val1 != 0) {
		hitObjAttrOn(val2, val3);
	} else {
		hitObjAttrOff(val2, val3);
	}
	return EVT_RETURN_DONE;
}

s32 name_callback_sub(const char* name, s32 callback, s32 flag) {
	// TODO: decompile 680B function
	return 0;
}

USER_FUNC(evt_hitobj_name_callback) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	name_callback_sub((const char*)val0, val1, 0);
	return EVT_RETURN_DONE;
}
