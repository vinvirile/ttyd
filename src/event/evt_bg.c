#include "evt/evt_bg.h"
#include "driver/bgdrv.h"

static GXColor unk_80422fe8;

USER_FUNC(evt_bg_disp_onoff) {
	s32* args = event->args;
	s32 flag = evtGetValue(event, args[0]);
	if (flag == 0) {
		bgDispOn();
	} else {
		bgDispOff();
	}
	return 2;
}

USER_FUNC(evt_bg_set_color) {
	s32* args = event->args;
	s32 r = evtGetValue(event, args[0]);
	s32 g = evtGetValue(event, args[1]);
	s32 b = evtGetValue(event, args[2]);
	s32 a = evtGetValue(event, args[3]);
	GXColor color = unk_80422fe8;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	bgSetColor(color);
	return 2;
}

USER_FUNC(evt_bg_auto_scroll_onoff) {
	s32* args = event->args;
	s32 flag = evtGetValue(event, args[0]);
	if (flag == 0) {
		bgAutoScrollOn();
	} else {
		bgAutoScrollOff();
	}
	return 2;
}

USER_FUNC(evt_bg_set_scrl_offset) {
	f32 x = evtGetFloat(event, event->args[0]);
	f32 y = evtGetFloat(event, event->args[1]);
	bgSetScrlOffset(x, y);
	return 2;
}
