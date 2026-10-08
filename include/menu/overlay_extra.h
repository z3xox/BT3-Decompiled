#ifndef MENU_OVERLAY_EXTRA_H
#define MENU_OVERLAY_EXTRA_H


/* overlay_common.h declares Snd_PlaySe as returning nothing; it returns s32 (the code after a call shows it). */
#define Snd_PlaySe Snd_PlaySe_menuA
#include "menu/overlay_common.h"
#undef Snd_PlaySe
extern s32 Snd_PlaySe(u32 mask, s32 id);

/* What the Evolution Z top menu and the options screen both take from outside their own files: more of the
   main executable than overlay_common.h declares, the pad bits as the menus use them, and the overlay's
   "save on leaving" helper. (One header with evo_top.h and the head of option.h until 2026-10-08.) */

/* ---- Main executable, beyond what overlay_common.h declares ---- */

extern void Sprite_SetScissor(s32 x0, s32 x1, s32 y0, s32 y1);
extern void Flash_ClipSetCallbackA(MFlash *flash, MFlashRef *ref, void *fn, void *arg);
extern void Flash_ClipSetCallbackB(MFlash *flash, MFlashRef *ref, void *fn, void *arg);
extern void StreamSe_FadeOutStep(s32 se);
extern void Bgm_Stop(void);
extern void Bgm_SetVolume(s32 volume);
extern void SndOpt_Apply(void);                       /* Bgm_SetVolume(0x40) + Snd_SetStereo(soundMode ^ 1) */
extern void BgmList_ApplyUnlocks(s32 *count, s32 *ids);

/* Voice_GetStat result when nothing is playing. */
#define MVOICE_IDLE 5

/* Pad.gamePressed / gameRepeat / gameHeld bits as the menus use them (sys/pad.h has the PADG_ names). */
#define MPAD_LEFT 1
#define MPAD_RIGHT 2
#define MPAD_DOWN 4
#define MPAD_UP 8
#define MPAD_OK 0x200
#define MPAD_CANCEL 0x400
#define MPAD_START 0x1000

/* ---- The overlay's "save on leaving" helper (0x3B0C08..0x3B0E04, chunk menu_za: DcSave) ---- */

extern void DcSave_Init(void *dialogPack);   /* Dialog_Init + McFlow_Init */
extern void DcSave_Term(void);
extern void DcSave_Start(void);              /* McFlow_Start(0) */
extern void DcSave_Update(void);             /* per frame: McFlow_Update while started */
extern s32 DcSave_GetState(void);            /* McFlow_Update's last result while started, else 0 */
extern s32 DcSave_IsDone(void);              /* 1 when not started, else whether the flow has ended */
extern s32 DcSave_IsStarted(void);

#endif
