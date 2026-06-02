#include "evt/evt_offscreen.h"
#include "driver/offscreendrv.h"

#include <dolphin/types.h>

extern OffscreenEntry* offscreenNameToPtr(const char* name);
extern s32 offscreenNameToId(const char* name);
extern void offscreenDelete(const char* name);
extern s32 offscreenGetBoundingBox_BeforeFrame(
    const char* name, u16* outLeft, u16* outTop, u16* outRight, u16* outBottom);

USER_FUNC(evt_offscreen_set_texture_fmt) {
	s32* args;
	const char* name;
	s32 fmt;
	s32 unk;
	OffscreenEntry* entry;

	args = event->args;
	name = (const char*)evtGetValue(event, args[0]);
	fmt = evtGetValue(event, args[1]);
	unk = evtGetValue(event, args[2]);
	entry = offscreenNameToPtr(name);
	entry->unk3C = fmt;
	entry->unk40 = unk;
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_offscreen_get_boundingbox2) {
	s32* args;
	const char* name;
	s32 a1;
	s32 a2;
	s32 a3;
	s32 a4;
	u16 out0;
	u16 out1;
	u16 out2;
	u16 out3;
	s32 result;

	args = event->args;
	name = (const char*)evtGetValue(event, args[0]);
	a1 = args[1];
	a2 = args[2];
	a3 = args[3];
	a4 = args[4];
	offscreenNameToId(name);
	out0 = 0;
	out1 = 0;
	out2 = 0;
	out3 = 0;
	if (isFirstCall) {
		event->userdata[0] = 5;
	}
	result = offscreenGetBoundingBox_BeforeFrame(name, &out0, &out1, &out2, &out3);
	event->userdata[0]--;
	if (result == 0 && event->userdata[0] != 0) {
		return 0;
	}
	evtSetValue(event, a1, out0);
	evtSetValue(event, a2, out1);
	evtSetValue(event, a3, (s32)out2 - (s32)out0);
	evtSetValue(event, a4, (s32)out3 - (s32)out1);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_offscreen_get_boundingbox) {
	s32* args;
	const char* name;
	s32 a1;
	s32 a2;
	s32 a3;
	s32 a4;
	u16 out0;
	u16 out1;
	u16 out2;
	u16 out3;
	s32 result;

	args = event->args;
	name = (const char*)evtGetValue(event, args[0]);
	a1 = args[1];
	a2 = args[2];
	a3 = args[3];
	a4 = args[4];
	offscreenNameToId(name);
	result = offscreenGetBoundingBox_BeforeFrame(name, &out0, &out1, &out2, &out3);
	if (result == 0) {
		return 0;
	}
	evtSetValue(event, a1, out0);
	evtSetValue(event, a2, out1);
	evtSetValue(event, a3, (s32)out2 - (s32)out0);
	evtSetValue(event, a4, (s32)out3 - (s32)out1);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_offscreen_delete) {
	s32* args;
	const char* name;

	args = event->args;
	name = (const char*)evtGetValue(event, args[0]);
	offscreenDelete(name);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_offscreen_entry) {
	s32* args;
	const char* name;

	args = event->args;
	name = (const char*)evtGetValue(event, args[0]);
	offscreenEntry(name);
	return EVT_RETURN_DONE;
}
