#include "evt/evt_lecture.h"
#include "manager/evtmgr.h"
#include "driver/camdrv.h"
#include "mario/mariost.h"
#include "mario/mario.h"
#include "mario/mario_party.h"
#include "party.h"

extern GlobalWork* gp;

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
	s32 a0 = evtGetValue(event, event->args[0]);
	CameraEntry* cam = camGetPtr(4);
	MarioWork* mp = marioGetPtr();
	PartyEntry* party = partyGetPtr(marioGetPartyId());
	PartyEntry* extra = partyGetPtr(marioGetExtraPartyId());

	*((u32*)((u8*)cam + 0x58)) = *((u32*)&cam->cameraPos.x);
	*((u32*)((u8*)cam + 0x5C)) = *((u32*)&cam->cameraPos.y);
	*((u32*)((u8*)cam + 0x60)) = *((u32*)&cam->cameraPos.z);
	*((u32*)((u8*)cam + 0x64)) = *((u32*)&cam->target.x);
	*((u32*)((u8*)cam + 0x68)) = *((u32*)&cam->target.y);
	*((u32*)((u8*)cam + 0x6C)) = *((u32*)&cam->target.z);
	*((u32*)((u8*)cam + 0x40)) = *((u32*)&stPt.x);
	*((u32*)((u8*)cam + 0x44)) = *((u32*)&stPt.y);
	*((u32*)((u8*)cam + 0x48)) = *((u32*)&stPt.z);
	*((u32*)((u8*)cam + 0x4C)) = *((u32*)&stAt.x);
	*((u32*)((u8*)cam + 0x50)) = *((u32*)&stAt.y);
	*((u32*)((u8*)cam + 0x54)) = *((u32*)&stAt.z);
	*((u32*)((u8*)cam + 0x70)) = *(u32*)((u8*)gp + 0x38);
	*((u32*)((u8*)cam + 0x74)) = *(u32*)((u8*)gp + 0x3C);

	{
		u32 nowScaled = (*(volatile u32*)0x800000F8 >> 2) / 1000;
		*((u32*)((u8*)cam + 0x7C)) = a0 * nowScaled;
		*((u32*)((u8*)cam + 0x78)) = 0;
	}
	cam->mode = 3;
	*((u8*)((u8*)cam + 0x80)) = 0xB;

	*((u32*)((u8*)mp + 0x8C)) = *((u32*)&stMpos.x);
	*((u32*)((u8*)mp + 0x90)) = *((u32*)&stMpos.y);
	*((u32*)((u8*)mp + 0x94)) = *((u32*)&stMpos.z);
	*(f32*)((u8*)mp + 0x1A8) = stMdir1;
	*(f32*)((u8*)mp + 0x1AC) = stMdir2;
	*(f32*)((u8*)mp + 0x1A0) = stMdir3;
	*(f32*)((u8*)mp + 0x19C) = stMdir4;

	if (party != NULL) {
		*((u32*)((u8*)party + 0x58)) = *((u32*)&stPpos.x);
		*((u32*)((u8*)party + 0x5C)) = *((u32*)&stPpos.y);
		*((u32*)((u8*)party + 0x60)) = *((u32*)&stPpos.z);
		*(f32*)((u8*)party + 0x10C) = stPdir1;
		*(f32*)((u8*)party + 0x110) = stPdir2;
		*(f32*)((u8*)party + 0x100) = stPdir3;
		*(f32*)((u8*)party + 0xFC) = stPdir4;
	}
	if (extra != NULL) {
		*((u32*)((u8*)extra + 0x58)) = *((u32*)&stXpos.x);
		*((u32*)((u8*)extra + 0x5C)) = *((u32*)&stXpos.y);
		*((u32*)((u8*)extra + 0x60)) = *((u32*)&stXpos.z);
		*(f32*)((u8*)extra + 0x10C) = stXdir1;
		*(f32*)((u8*)extra + 0x110) = stXdir2;
		*(f32*)((u8*)extra + 0x100) = stXdir3;
		*(f32*)((u8*)extra + 0xFC) = stXdir4;
	}
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

USER_FUNC(lect_test_systemlevel) {
	marioStGetSystemLevel();
	return EVT_RETURN_DONE;
}

USER_FUNC(lect_set_systemlevel) {
	s32 val = evtGetValue(event, event->args[0]);
	marioStSystemLevel(val);
	return EVT_RETURN_DONE;
}
