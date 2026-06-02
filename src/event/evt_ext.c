#include "evt/evt_ext.h"

extern void extReset(void);
extern void extEntry(s32 p0, s32 p1, s32 p2, s32 p3, s32 p4);

USER_FUNC(evt_ext_entry) {
	s32* args = event->args;
	s32 v0 = evtGetValue(event, args[0]);
	s32 v1 = evtGetValue(event, args[1]);
	s32 v2 = evtGetValue(event, args[2]);
	s32 v3 = evtGetValue(event, args[3]);
	s32 v4 = evtGetValue(event, args[4]);
	extEntry(v0, v1, v2, v3, v4);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_ext_reset) {
	extReset();
	return EVT_RETURN_DONE;
}
