#include "evt/evt_paper.h"
#include "mario/mariost.h"
#include "driver/animdrv.h"

extern GlobalWork* gp;

USER_FUNC(evt_paper_entry) {
	const char* name = (const char*)evtGetValue(event, event->args[0]);
	s32 sign = ((u32)(-(s32)gp->inBattle) | (u32)gp->inBattle) >> 31;
	s32 r = animGroupBaseAsync(name, sign, NULL);
	if (r == 0) {
		return 0;
	}
	animPaperPoseEntry(name, sign);
	return EVT_RETURN_DONE;
}

USER_FUNC(evt_paper_delete) {
	const char* name = (const char*)evtGetValue(event, event->args[0]);
	s32 sign = ((u32)(-(s32)gp->inBattle) | (u32)gp->inBattle) >> 31;
	animPaperPoseRelease(animPaperPoseGetId(name, sign));
	return EVT_RETURN_DONE;
}
