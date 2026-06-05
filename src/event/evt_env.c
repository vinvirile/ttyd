#include "evt/evt_env.h"
#include "driver/envdrv.h"
#include "manager/evtmgr.h"

USER_FUNC(evt_env_blur_on) {
	s32* args = event->args;
	s32 val0 = evtGetValue(event, args[0]);
	s32 val1 = evtGetValue(event, args[1]);
	u64 lifetime = event->lifetime;

	if (isFirstCall) {
		envBlurOn(val0, val1);
		*(s32*)((char*)event + 0x78) = (s32)((lifetime >> 32) & 0xFFFFFFFFu);
		*(s32*)((char*)event + 0x7C) = (s32)(lifetime & 0xFFFFFFFFu);
	}

	if (val1 == 0) goto done;
	{
		u32 n = *(volatile u32*)0x800000F8;
		u32 nowScaled = (n >> 2) / 1000;
		u64 lifetimeDiff = lifetime - (((u64)(u32)event->userdata[0] << 32) | (u32)event->userdata[1]);
		u64 elapsed = lifetimeDiff / nowScaled;
		if (elapsed >= (u64)val1) {
			envBlurOff();
			return EVT_RETURN_DONE;
		}
	}
done:
	return 0;
}
