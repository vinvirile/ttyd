#include "mario/mario.h"
#include "mario/mario_party.h"
#include "party.h"

extern void bero_clear_Offset(void);
extern void psndSFXOff(s32 id);

//.rodata
static const f32 vec3_802f32e0[3] = { 0.0f, 0.0f, 0.0f };
static const f32 vec3_802f32ec[3] = { 0.0f, 0.0f, 0.0f };

//.sdata2
static const f32 float_0_8041c8d8 = 0.0f;

void mot_yoshi_post(void) {
	PartyEntry* party;
	MarioWork* mario;
	s32 unk168;

	party = partyGetPtr(marioGetPartyId());
	mario = marioGetPtr();
	unk168 = party->field_0x168[0];
	if ((u32)(unk168 + 0x10000) != 0xFFFFu) {
		psndSFXOff(unk168);
		party->field_0x168[0] = -1;
	}
	bero_clear_Offset();
	mario->dispFlags &= ~0x20000000;
	mario->dispFlags &= ~0x00080000;
	((s32*)((char*)mario + 0x98))[0] = ((s32*)vec3_802f32e0)[0];
	((s32*)((char*)mario + 0x9C))[0] = ((s32*)vec3_802f32e0)[1];
	((s32*)((char*)mario + 0xA0))[0] = ((s32*)vec3_802f32e0)[2];
	((s32*)((char*)mario + 0xA4))[0] = ((s32*)vec3_802f32ec)[0];
	((s32*)((char*)mario + 0xA8))[0] = ((s32*)vec3_802f32ec)[1];
	((s32*)((char*)mario + 0xAC))[0] = ((s32*)vec3_802f32ec)[2];
}

void mot_yoshi(void) {
	MarioWork* mario;

	mario = marioGetPtr();
	if (mario->trigFlags & 1) {
		mario->trigFlags &= ~1;
		mario->flags &= ~0xF000;
		mario->dispFlags |= 0x4;
		mario->dispFlags |= 0x1000;
		*(s32*)((char*)mario + 0x48) = 0;
		*(s16*)((char*)mario + 0x50) = 0;
		mario->currSubMotionId = 0;
		*(f32*)((char*)mario + 0x180) = 0.0f;
		*(s32*)((char*)mario + 0x1DC) = 0;
		*(s32*)((char*)mario + 0x1E0) = 0;
		*(s32*)((char*)mario + 0x1E4) = 0;
		*(s32*)((char*)mario + 0x1EC) = 0;
		*(s32*)((char*)mario + 0x1F0) = 0;
		*(s32*)((char*)mario + 0x1F4) = 0;
		*(s32*)((char*)mario + 0x1F8) = 0;
	}
}
