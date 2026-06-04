#include "evt/evt_lecture.h"
#include "manager/evtmgr.h"
#include "driver/camdrv.h"
#include "mario/mariost.h"

//.sbss
s32 lect_work;
Vec stXdir4;
Vec stXdir3;
Vec stXdir2;
Vec stXdir1;
Vec stPdir4;
Vec stPdir3;
Vec stPdir2;
Vec stPdir1;
Vec stMdir4;
Vec stMdir3;
Vec stMdir2;
Vec stMdir1;
Vec stXpos;
Vec stPpos;
Vec stMpos;
Vec stAt;
Vec stPt;

USER_FUNC(lect_juyoitem_rel_get) {
	s32* args = event->args;
	evtSetValue(event, args[0], lect_work);
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_juyoitem_rel_set) {
	s32* args = event->args;
	lect_work = evtGetValue(event, args[0]);
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_cam_load) {
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_cam_save) {
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_test_systemlevel) {
	marioStGetSystemLevel();
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_set_systemlevel) {
	s32 val = evtGetValue(event, event->args[0]);
	marioStSystemLevel(val);
	return EVT_RETURN_DONE;
}
