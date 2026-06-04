#include "driver/effdrv.h"
#include "driver/dispdrv.h"
#include "driver/camdrv.h"
#include "driver/animdrv.h"
#include "mario/mariost.h"
#include "memory.h"

#include <dolphin/mtx.h>

extern GlobalWork* gp;
extern void animPoseDrawMtx(s32 poseId, Mtx matrix, s32 xluMode, f32 rotY, f32 scale);
extern f32 animPoseGetLoopTimes(s32 poseId);

typedef struct KumokumoChipN64Data {
	s32 kind;
	f32 posX;
	f32 posY;
	f32 posZ;
	f32 scale;
	s32 poseId;
	s32 state;
	s32 pad;
	s32 alpha;
} KumokumoChipN64Data;

void effKumokumoChipDisp(CameraId camId, void* param) {
	EffectEntry* entry = (EffectEntry*)param;
	KumokumoChipN64Data* data = (KumokumoChipN64Data*)entry->userdata;

	if (data->poseId != -1) {
		Mtx mtx, scaleMtx, rotMtx, tmpMtx;

		PSMTXTrans(mtx, data->posX, data->posY, data->posZ);
		PSMTXScale(scaleMtx, data->scale, data->scale, data->scale);

		{
			CameraEntry* cam = camGetPtr(camId);
			f32 deg = -cam->viewYaw;
			f32 rad = deg * 0.017453292f;
			PSMTXRotRad(rotMtx, 'y', rad);
		}

		PSMTXConcat(mtx, rotMtx, tmpMtx);
		PSMTXConcat(tmpMtx, scaleMtx, mtx);

		animPoseSetMaterialFlagOn(data->poseId, 0x40);

		{
			s32 alpha = data->alpha;
			u32 col = 0xFFFFFF00;
			u8* colBytes = (u8*)&col;
			colBytes[3] = alpha;
			animPoseSetMaterialEvtColor(data->poseId, (GXColor*)&col);
		}

		animPoseMain(data->poseId);
		animPoseDrawMtx(data->poseId, mtx, 1, 0.0f, 10.0f);
		animPoseDrawMtx(data->poseId, mtx, 2, 0.0f, 10.0f);
		animPoseDrawMtx(data->poseId, mtx, 3, 0.0f, 10.0f);
	}
}

void effKumokumoChipMain(EffectEntry* entry) {
	KumokumoChipN64Data* data = (KumokumoChipN64Data*)entry->userdata;
	Vec pos;

	pos.x = data->posX;
	pos.y = data->posY;
	pos.z = data->posZ;

	if (entry->flags & 8) {
		entry->flags &= ~8;
		if (data->poseId != -1) {
			animPoseRelease(data->poseId);
		}
		effDelete(entry);
		return;
	}

	switch (data->state) {
	case 0:
	{
		s32 isInBattle = ((u32)(-(s32)gp->inBattle) | (u32)gp->inBattle) >> 31;
		s32 r = animGroupBaseAsync("KumokumoChipN64", isInBattle, NULL);
		if (r != 0) {
			s32 pose = animPoseEntry("KumokumoChipN64", isInBattle);
			data->poseId = pose;
			animPoseSetAnim(pose, "A_1", 1);
			data->state++;
		}
		break;
	}
	case 1:
	{
		f32 loop = animPoseGetLoopTimes(data->poseId);
		if (loop > 1.0f) {
			animPoseSetAnim(data->poseId, "S_1", 0);
			data->state++;
		}
		break;
	}
	}

	dispCalcZ(&pos);
	dispEntry(4, 2, effKumokumoChipDisp, entry, pos.z);
}

EffectEntry* effKumokumoChipN64Entry(s32 kind, f32 x, f32 y, f32 z) {
	EffectEntry* entry = effEntry();
	KumokumoChipN64Data* data;

	entry->type = "KumokumoChipN64";
	entry->count = 1;
	data = (KumokumoChipN64Data*)__memAlloc(HEAP_EFFECT, sizeof(KumokumoChipN64Data));
	entry->userdata = data;
	entry->callback = effKumokumoChipMain;

	entry->flags |= 2;

	data->kind = kind;
	data->posX = x;
	data->posY = y;
	data->posZ = z;
	data->scale = 1.0f;
	data->poseId = -1;
	data->state = 0;
	data->pad = 0;
	data->alpha = 0xFF;

	return entry;
}
