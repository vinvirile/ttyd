#include "evt/evt_snd.h"
#include "pmario_sound.h"
#include "win/win_main.h"
#include "mario/mariost.h"

void L_camDispOn(s32 arg);
void L_psndBGM_stop(void);
void N_pouchUnEquipBadgeID(s32 badgeId);
void psndSetReverb(s32 reverb);

extern PaperSoundBGM psbgm;
extern u8 sound[]; // 0x15C4 bytes, accessed at +0x100 as u16 array with 0x138 stride

void seq_gameOverInit(void) {
	winReInit();
	L_camDispOn(4);
	psndStopAllFadeOut();
	psndSetReverb(0);
	L_psndBGM_stop();
	psndClearFlag(0x80);
	psndClearFlag(0x20);
	psndClearFlag(0x40);
	N_pouchUnEquipBadgeID(0x143);
	marioStSystemLevel(4);
}

void seq_gameOverExit(void) {
	psndStopAllFadeOut();
	marioStSystemLevel(0);
}

USER_FUNC(N_evt_snd_bgm_unpause) {
	s32 streamId = psbgm.streamId;
	if ((u32)(streamId + 0x10000) != 0xFFFFu) {
		void* table = (void*)*(u32*)(sound + 0x100);
		u16* p = (u16*)((char*)table + streamId * 0x138);
		*p = *p & 0xFFF0;
	}
	return EVT_RETURN_DONE;
}
