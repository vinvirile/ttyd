#include "evt/evt_light.h"
#include "driver/lightdrv.h"
#include "driver/animdrv.h"
#include "driver/mapdrv.h"
#include "driver/npcdrv.h"
#include "mario/mario_party.h"
#include "mario/mariost.h"
#include "party.h"

USER_FUNC(evt_light_charalight_all_onoff) {
	s32 on = evtGetValue(event, event->args[0]);
	s32 i;
	if (on != 0) {
		for (i = 0; i < lightGetEntryNum(); i++) {
			LightEntry* entry = lightNumberToPtr(i);
			entry->flags |= LIGHT_CHARA;
		}
	} else {
		for (i = 0; i < lightGetEntryNum(); i++) {
			LightEntry* entry = lightNumberToPtr(i);
			entry->flags &= ~LIGHT_CHARA;
		}
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_light_mario_lighting_onoff) {
	s32 on = evtGetValue(event, event->args[0]);
	MarioWork* mp = marioGetPtr();
	if (on != 0) {
		animPoseSetMaterialLightFlagOn(mp->paperAnimGroupId[0], 2);
		animPoseSetMaterialLightFlagOn(mp->paperAnimGroupId[1], 2);
		animPoseSetMaterialLightFlagOn(mp->paperAnimGroupId[2], 2);
	} else {
		animPoseSetMaterialLightFlagOff(mp->paperAnimGroupId[0], 2);
		animPoseSetMaterialLightFlagOff(mp->paperAnimGroupId[1], 2);
		animPoseSetMaterialLightFlagOff(mp->paperAnimGroupId[2], 2);
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_light_party_lighting_onoff) {
	s32 on = evtGetValue(event, event->args[0]);
	PartyEntry* party = partyGetPtr(marioGetPartyId());
	if (on != 0) {
		animPoseSetMaterialLightFlagOn(party->field_0xC, 2);
	} else {
		animPoseSetMaterialLightFlagOff(party->field_0xC, 2);
	}
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_light_npc_lighting_onoff) {
	s32* args = event->args;
	s32 on = evtGetValue(event, args[0]);
	NpcEntry* npc = npcNameToPtr((const char*)evtGetValue(event, args[1]));
	if (on != 0) {
		animPoseSetMaterialLightFlagOn(npc->poseId, 2);
	} else {
		animPoseSetMaterialLightFlagOff(npc->poseId, 2);
	}
	return EVT_RETURN_DONE;
}

//.rodata
static const Vec vec3_802f3148 = {0, 0, 0};
static const Vec vec3_802f3154 = {0, 0, 0};

USER_FUNC(evt_light_bind_mapobj) {
	s32* args = event->args;
	const char* lightName = (const char*)evtGetValue(event, args[0]);
	const char* mapObjName = (const char*)evtGetValue(event, args[1]);
	s32 matrixMode = evtGetValue(event, args[2]);
	LightEntry* light = lightNameToPtr(lightName);
	MapObject* mapObj = mapGetMapObj(mapObjName);
	Vec* storedDir = (Vec*)&event->userdata[0];

	if (isFirstCall) {
		Vec zero = vec3_802f3148;
		Vec mapObjPos;
		PSMTXMultVec(mapObj->modelWorldMtx, &zero, &mapObjPos);
		PSVECSubtract(&light->field_0x3C, &mapObjPos, storedDir);
	}

	{
		Vec dir;
		dir.x = storedDir->x;
		dir.y = storedDir->y;
		dir.z = storedDir->z;
		switch (matrixMode) {
		case 0:
			PSMTXMultVec(mapObj->unkAC, &dir, &dir);
			break;
		case 1: {
			Mtx mtx;
			PSMTXTrans(mtx, mapObj->unkAC[0][3], mapObj->unkAC[1][3], mapObj->unkAC[2][3]);
			PSMTXMultVec(mtx, &dir, &dir);
			break;
		}
		default:
			break;
		}

		{
			Vec zero = vec3_802f3154;
			Vec worldDir;
			PSMTXMultVec(mapObj->modelWorldMtx, &zero, &worldDir);
			PSVECAdd(&worldDir, &dir, &light->position);
		}
	}
	return EVT_RETURN_BLOCK;
}
