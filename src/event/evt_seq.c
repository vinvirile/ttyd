#include "evt/evt_seq.h"
#include "driver/seqdrv.h"

USER_FUNC(evt_seq_set_seq) {
	s32* args = event->args;
	s32 seq = evtGetValue(event, args[0]);
	s32 map = evtGetValue(event, args[1]);
	s32 bero = evtGetValue(event, args[2]);
	seqSetSeq(seq, (const char*)map, (const char*)bero);
	return 0;
}

USER_FUNC(evt_seq_wait) {
	s32* args = event->args;
	s32 val = evtGetValue(event, args[0]);
	s32 cur = seqGetSeq();
	s32 diff = (cur - val) | (val - cur);
	return diff < 0 ? 0 : 2;
}
