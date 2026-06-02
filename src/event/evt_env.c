#include "evt/evt_env.h"
#include "driver/envdrv.h"
#include "manager/evtmgr.h"

USER_FUNC(evt_env_blur_on) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	u32 lifetimeHi = (u32)(event->lifetime >> 32);
	u32 lifetimeLo = (u32)event->lifetime;
	s32 val1 = evtGetValue(event, args[1]);

	if (isFirstCall) {
		envBlurOn(val0, val1);
		event->userdata[0] = (s32)lifetimeHi;
		event->userdata[1] = (s32)lifetimeLo;
	}

	if (val1 != 0) {
		u32 n = *(volatile u32*)0x800000F8;
		u32 nowScaled = (u32)(((u64)(u32)(n >> 2) * (u64)0x10624DD3u) >> 38);
		u64 lifetimeDiff = event->lifetime - ((u64)(u32)event->userdata[0] << 32 | (u32)event->userdata[1]);
		u64 elapsed = lifetimeDiff / nowScaled;
		if ((s64)elapsed < (s64)val1) {
			return 0;
		}
		envBlurOff();
		return EVT_RETURN_DONE;
	}
	return 0;
}
