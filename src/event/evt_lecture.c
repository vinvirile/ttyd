#include "evt/evt_lecture.h"
#include "manager/evtmgr.h"
#include "driver/camdrv.h"
#include "mario/mariost.h"
#include "mario/mario.h"
#include "mario/mario_party.h"
#include "party.h"

//.sbss
s32 lect_work;
f32 stXdir4;
f32 stXdir3;
f32 stXdir2;
f32 stXdir1;
f32 stPdir4;
f32 stPdir3;
f32 stPdir2;
f32 stPdir1;
f32 stMdir4;
f32 stMdir3;
f32 stMdir2;
f32 stMdir1;
Vec stXpos;
Vec stPpos;
Vec stMpos;
Vec stAt;
Vec stPt;

USER_FUNC(lect_set_systemlevel) {
	s32 val = evtGetValue(event, event->args[0]);
	marioStSystemLevel(val);
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_test_systemlevel) {
	marioStGetSystemLevel();
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_cam_save) {
	CameraEntry* cam = camGetPtr(4);
	MarioWork* mp = marioGetPtr();
	PartyEntry* party = partyGetPtr(marioGetPartyId());
	PartyEntry* extra = partyGetPtr(marioGetExtraPartyId());

	*((u32*)&stPt.x) = *((u32*)&cam->cameraPos.x);
	*((u32*)&stPt.y) = *((u32*)&cam->cameraPos.y);
	*((u32*)&stPt.z) = *((u32*)&cam->cameraPos.z);
	*((u32*)&stAt.x) = *((u32*)&cam->target.x);
	*((u32*)&stAt.y) = *((u32*)&cam->target.y);
	*((u32*)&stAt.z) = *((u32*)&cam->target.z);
	*((u32*)&stMpos.x) = *(u32*)((u8*)mp + 0x8C);
	*((u32*)&stMpos.y) = *(u32*)((u8*)mp + 0x90);
	*((u32*)&stMpos.z) = *(u32*)((u8*)mp + 0x94);
	stMdir1 = *(f32*)((u8*)mp + 0x1A8);
	stMdir2 = *(f32*)((u8*)mp + 0x1AC);
	stMdir3 = *(f32*)((u8*)mp + 0x1A0);
	stMdir4 = *(f32*)((u8*)mp + 0x19C);
	if (party != NULL) {
		*((u32*)&stPpos.x) = *(u32*)((u8*)party + 0x58);
		*((u32*)&stPpos.y) = *(u32*)((u8*)party + 0x5C);
		*((u32*)&stPpos.z) = *(u32*)((u8*)party + 0x60);
		stPdir1 = *(f32*)((u8*)party + 0x10C);
		stPdir2 = *(f32*)((u8*)party + 0x110);
		stPdir3 = *(f32*)((u8*)party + 0x100);
		stPdir4 = *(f32*)((u8*)party + 0xFC);
	}
	if (extra != NULL) {
		*((u32*)&stXpos.x) = *(u32*)((u8*)extra + 0x58);
		*((u32*)&stXpos.y) = *(u32*)((u8*)extra + 0x5C);
		*((u32*)&stXpos.z) = *(u32*)((u8*)extra + 0x60);
		stXdir1 = *(f32*)((u8*)extra + 0x10C);
		stXdir2 = *(f32*)((u8*)extra + 0x110);
		stXdir3 = *(f32*)((u8*)extra + 0x100);
		stXdir4 = *(f32*)((u8*)extra + 0xFC);
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_cam_load) {
	CameraEntry* cam = camGetPtr(4);
	MarioWork* mp = marioGetPtr();
	PartyEntry* party = partyGetPtr(marioGetPartyId());
	PartyEntry* extra = partyGetPtr(marioGetExtraPartyId());
	s32 a0 = evtGetValue(event, event->args[0]);

	cam->cameraPos.x = *((f32*)&stPt.x);
	cam->cameraPos.y = *((f32*)&stPt.y);
	cam->cameraPos.z = *((f32*)&stPt.z);
	cam->target.x = *((f32*)&stAt.x);
	cam->target.y = *((f32*)&stAt.y);
	cam->target.z = *((f32*)&stAt.z);

	if (extra != NULL) {
		// TODO
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_juyoitem_rel_set) {
	s32* args = event->args;
	lect_work = evtGetValue(event, args[0]);
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_juyoitem_rel_get) {
	s32* args = event->args;
	evtSetValue(event, args[0], lect_work);
	return EVT_RETURN_DONE;
}
