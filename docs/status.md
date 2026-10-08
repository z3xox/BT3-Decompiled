# Current status (handoff note)

## State on 2026-10-08 (branch `asm-left`, on top of `rename`): no game function is left as INCLUDE_ASM

- Both outputs are byte-identical to the disc's from a fresh tree (`rm -rf build asm`, configure, ninja).
- `scripts/progress.py`: main executable 99.92% of the game code in C, menu overlay 99.97%; 0 INCLUDE_ASM lines
  in either. The 45 functions that were still assembly on 2026-10-07 all match in C now, `ColObb_Contact`
  (4,257 instructions) among them. What each needed is in docs/decomp_guide.md, "Lessons from the night of
  2026-10-08".
- Still assembly, and meant to be: the four hand-written VU0 routines (`ObjSeam_TransformVtx` in `cod/00FFD0`;
  `StgVu_RotateZ` / `X` / `Y` in `cod/140C68`, which is also why `stg.c` and `stg_parts.c` are two files), and the
  libraries (Sony, CRI, libc) behind 0x273CF0.
- Fake matches made that night (byte-identical, a statement that emits nothing stands where the real source form
  is not known; each is marked `FAKE MATCH` in the source): `ObjShadow_BuildPacket`, `DemoCam_Update`,
  `EftStreak_DrawScreen`, `EftStreak_Draw` (uses `register ... __asm__("$0")`: PS2 compiler only),
  `EftBlade_AddPoint`, `Shen_DrawList`, `StgDepthTint_Draw`, `EftWater_DrawClippedFan`; possible stand-ins:
  the width variable in `DcPass_DrawStatus`, `base = &p0;` in `EftBolt_Shape`.
- The files were renamed and some merged on the branch `rename` (docs/rename_draft.md,
  docs/file_rename_map.txt). Everything below this section, and the other documents, use the OLD file names and
  speak of functions as INCLUDE_ASM that no longer are: read them as history.
- Corrections that came out of the matches: `sceGsSetDefStoreImage` and `Mtx_ProjectPointStq` return s32;
  `ScrXfade_Start(f32 seconds, s32 request)`; `Vec3_ScaleAdd(dst, dir, f32 s, base)` in eft_stage_1.c;
  `EftGndDust_DrawQuad` / `DrawPiece` parameter orders; `gSimTrain1` / `gSimTrain2` are const; `Col_NearEq` is
  const (inferred: static in the original); the spawners of eft_emit.c take their floats first.
- Not done: the port repository has none of this (renames, merges, the 45 functions).


Written 2026-10-04 so the work can be picked up without the conversation that produced it.
Update or delete when it goes stale.

## Verified state

- Last build verified byte-identical: the integration described in "Ninth step" at the end of this
  file (75.62% of the main executable's game code; 171 linked C files; 5877 functions diff clean;
  166 INCLUDE_ASM in linked files; DBZP 0%). Not committed by the integrator: commit after checking.
  The commit before it is the cleanup link (69.83%).
- Check at any time: `.venv/bin/python configure.py && ninja`, then
  `cmp build/SLUS_216.78.rom disc/SLUS_216.78.rom` and `cmp build/DBZP.BIN disc/BIN/DBZP.BIN`,
  then `python3 scripts/progress.py`.

## Fighter-core batch: linked

Script commands, stats and animation, hits and collision, members, input conditions and
control, flags and clashes, movement, the action core, and the AI's virtual pad, sense and
rule evaluator are all linked. Merges made at integration: `btl_char_coll.c` into
`btl_char_hit.c`; `btl_char_ctl.c` into `btl_input.c`; `btl_ai_think.c` into `btl_ai_cond.c`
(`BtlAi_ScaleByLevel` moved to the end of `btl_ai_seq.c`); `BtlColl_NextPoolMember` moved to
the top of `btl_char_member.c`. Seven functions of that batch stay INCLUDE_ASM
(`BtlAiSense_IsBusy` and six `AiThink_*`).

## Action-handler batch: linked

The fighter effect layer, the action handlers, the fighter API and the technique / parameter readers
(0x1CF578..0x1D3B40 and 0x1E3158..0x2129C8, all of it) are linked and the build is byte-identical:
91 C files, 2887 functions diff clean, 34.54% of the main executable's game code in C.

Files as linked (src/battle/), with the merges made at integration:

| File | Range | Notes |
|---|---|---|
| btl_char_member.c | 0x1CDCA8..0x1CF578 | lost its tail (the effect request bits) to btl_char_fx.c |
| btl_char_fx.c | 0x1CF578..0x1D1EC8 | the tail of btl_char_member.c + btl_char_fx.c + btl_char_fx_b.c: `BtlFx_UpdateGroundFx` needs `BtlChar_IsFxBitNew` defined in its file and now matches in C. `BtlFx_SpawnSpeedLines` (four LIT4_WORD) and `BtlFx_FireKiBlast` (formerly `BtlFx_SpawnDamageSparks`; last function, RODATA_ALIGN16; its constants 0x2FD200..0x2FD214 are the assembly chunk `cod/1FD200`) stay INCLUDE_ASM |
| btl_char_fx_c.c | 0x1D1EC8..0x1D3B40 | unchanged |
| btl_act_a.c | 0x1E3158..0x1EA5F8 | btl_act_a.c + btl_act_b.c: `BtlAct_AttackDashHandler` now matches in C. `BtlActB_TickMemberChange` stays INCLUDE_ASM. Emits the 8 bytes of `.sdata` at 0x2FEB20. Not merged into btl_char_action.c (nothing needs it; it would be a third fighter view in one file) |
| btl_act_c.c, _d.c, _e.c | 0x1EA5F8.., 0x1EE058.., 0x1F1930..0x1F5460 | as written; `BtlAct_GuardHandler` INCLUDE_ASM |
| btl_act_f.c | 0x1F5460..0x1FC2B0 | btl_act_f.c + btl_act_g.c: `BtlAct_SuperRushDashHandler` matches in C once it is in one file with the first part (found at integration). `BtlAct_SuperRushFollowHandler` carries a redundant prototype as a matching aid (see the comment there and decomp_guide.md) |
| btl_act_h.c | 0x1FC2B0..0x1FC598 | `BtlAct_GrabDash` INCLUDE_ASM; its constants are the assembly chunk `cod/1FDEB4` |
| btl_act_h_b.c | 0x1FC598..0x203168 | btl_act_h_b.c + btl_act_i.c: `BtlAct_SwitchArriveLand` and `BtlAct_KoSwitchFlyIn` now match in C |
| btl_act_j.c | 0x203168..0x204E78 | as written; its float pool is 0x2FE070..0x2FE07C (three constants, not two) |
| btl_capi_a.c | 0x204E78..0x207020 | `BtlCharApi_HasKiBlastType2/3` INCLUDE_ASM; float pool 0x2FE07C..0x2FE0CC |
| btl_capi_b.c | 0x208430..0x20BA80 | as written |
| btl_tech_a.c, btl_tech_b.c | 0x20BA80..0x20F0E8..0x2129C8 | as written |

In a merged file each part keeps its own header and its own view of the fighter; functions the first part
already declared are reached from the second part through cast macros
(`#define Name ((ret (*)(args))Name)`), which leave the generated code unchanged.

Seven functions of this batch stay INCLUDE_ASM (docs/open_questions.md).

Evidence of original file boundaries not acted on: btl_char_action.c + btl_act_a.c are one object (the
float pool and jump tables run on); btl_act_f.c's last two functions (from 0x1FC008) probably belong with
btl_act_h.c; where the object holding btl_char_fx.c starts (0x1CF578 is the latest possible place).

`scripts/apply_names.py` reads every file in config/symbols/, including those of agents whose
files are not in the yamls yet, and its skip arguments only skip the source files it rewrites, not the
symbol files it reads: while unlisted symbol files exist, run a copy that ignores them.

## Effect, stage, collision, battle-object batch: linked

Everything decompiled so far is linked: the tree has NO unlinked C file and every file in config/symbols/ is
listed in both yamls (so `scripts/apply_names.py` can be run as is). Findings are in docs/systems/; the
per-file tables of the two effect waves are in the history of this file (commits "Link the first effects wave
and the stage" and "Link the second effects wave ...").

Ranges in C (src/battle/ unless noted), with the merges made at integration:

| Range | Files | Notes |
|---|---|---|
| 0x12DD80..0x1637A0 | eft_a .. eft_m | first effect wave. Merged: the head of eft_e.c into eft_d_b.c (`EftBurst_Update`), eft_f.c into eft_e.c (`EftWaterRing_Update`), eft_l.c into eft_k.c (`EftRushShot_UpdateAttached`) |
| 0x1637A0..0x1AA7E8 | eft_n .. eft_ad_c (around btl_pool.c) | second effect wave. Merged: eft_t.c into eft_s.c, eft_u.c into eft_t_c.c, eft_v.c into eft_u_b.c, the tail of eft_y.c into eft_z.c, eft_ac.c into eft_ab_c.c. Four attempts are compiled as `ASM_STUB` definitions for their callers |
| 0x1AA7E8..0x1AE2A8 | eft_ae.c | sprite animations, the task tree, effect texture VRAM. As written; 3 INCLUDE_ASM. `.lit4` 0x2FCF04..0x2FCF14 (the chunk `cod/1FCEEC` in front holds the constants of eft_ad_b / eft_ad_c INCLUDE_ASM functions). No `.rodata`. Not merged with the head of eft_det_a.c: `EftTexSet_CheckCount` (0x1AE140, empty) is declared `__attribute__((const))` in both files and everything matches |
| 0x1AE2A8..0x1B4140 | eft_det_a, eft_det_b, eft_det_b_b, eft_det_b_c | hit detection, stage collision; eft_det_b_c.c is the head of the AI sequence object and was left a file of its own (merging it into btl_ai_seq.c changes nothing: `BtlAiSeq_PushRule` still differs by 66 of 85) |
| 0x1B4140..0x1B6D50 | btl_ai_seq.c | btl_ai_seq_a.c merged in, replacing the 38 INCLUDE_ASM lines; the table `D_002ED970` is now a local initialiser. 2 INCLUDE_ASM (`BtlAiStep_GuardUntilSafe`, `BtlAiStep_Unk17`). `.rodata` 0x2ED8C0 (0x1B0 bytes), `.lit4` 0x2FCFDC..0x2FCFF8 (the former chunk `cod/1FCFDC` was this file's constants, not eft_det_b's) |
| 0x22FD10..0x236190, 0x115170..0x115478 | stg_d.c, col_a.c, stg_d_b.c | rigid bodies, collision primitives part one |
| 0x236190..0x239BB0 | col_b.c | collision primitives part two, as written, no INCLUDE_ASM. `.lit4` 0x2FE48C..0x2FE4B8, `.sdata` 0x2FEBAC (4 bytes, the compiler-pooled 0x7F7FFFFE) |
| 0x239BB0..0x23C310 | col_c.c | the text printer: col_b_b.c (packet helpers) merged into the top, `Font_Flush` now matches in C. No INCLUDE_ASM. `.rodata` 0x2F21A0..0x2F2220 (four jump tables), `.lit4` 0x2FE4B8 (one constant); 0x2FE4BC..0x2FE4CC is `DemoCam_Update`'s (chunk `cod/1FE4BC`) |
| 0x23C310..0x23D1E8 | col_c_b.c | inline icon tags; 1 INCLUDE_ASM (`FontIcon_PutSprite`), no data |
| 0x23FB20..0x248F28 | stg_a, stg_a_b, stg_b, stg_c (and the assembly chunk `cod/140C68`, VU0 macro code) | stage |
| 0x24BBE8..0x250B28 | bobj_a.c | bobj_a.c + bobj_b.c: `BtlObj_UpdateFace` and `BtlObj_IsJawActive` now match in C (they need `BtlObjFace_StepBlink` 0x24EB70 / `BtlObjMdl_HasJaw` 0x24E9C8 defined above them; verified with the real bodies). 5 INCLUDE_ASM. `.rodata` 0x2F2420..0x2F2AA0 (the table `D_002F2420` by INCLUDE_RODATA, then the jump tables), `.lit4` 0x2FE640..0x2FE67C (two LIT4_WORD) |
| 0x250B28..0x2527B0 | bobj_b_b.c | secondary motion chains, as written; 3 INCLUDE_ASM. `.lit4` 0x2FE67C..0x2FE718 (28 LIT4_WORD); `BObjChainA_Step`'s 19 constants 0x2FE718..0x2FE764 are the chunk `cod/1FE718`. The tables 0x2F2AA0..0x2F2EC0 stay the chunk `cod/1F2AA0` |

Functions still INCLUDE_ASM in linked files: 185 (183 with a C attempt next to them), listed in
docs/open_questions.md.

Names: `eft_ae.txt`, `col_b.txt`, `col_c.txt`, `bobj_a.txt`, `bobj_b.txt`, `btl_ai_seq_a.txt` were listed last; no
duplicate names or addresses in any symbol file. The name pass replaced 1233 placeholder uses in 111 linked
files (`BtlTask_*`, `EftVram_*`, `BtlObjAnim_*`, `BtlObjXf_*`, `BtlObj_GetNode`, `Font_*`, `ColCapsule_Set`,
`ColSphere_Set`, the AI sequence handlers ..). No wrong callee name turned up at link.

Prelude: `__gp_forget` in include/gcc_prelude.inc uses `.set mips64` since the first effect wave (see
decomp_guide.md).

Evidence of object boundaries not acted on: eft_v_b.c .. eft_y.c (0x1871A8..0x195038) may be one object;
0x1ADBA8..0x1AE5F8 (the tail of eft_ae.c and the head of eft_det_a.c) is one source file; eft_det_b_c.c +
btl_ai_seq.c are one object starting at 0x1B3F78; stg_d may extend back to 0x22FC40; col_a probably starts at
0x230B10; eft_ae.c holds three objects (cuts at 0x1AA818, 0x1AD138, 0x1ADBA8) and its first function belongs to
eft_ad_c.c's object.

Not decompiled and not assigned: 0x22FC40..0x22FD10 (two stage rigid list helpers plus one function of another
module).

## Known follow-ups

- docs/systems/ still uses the old names of the function and task flags renamed at the second effects
  integration (`BtlFx_SpawnDamageSparks`, `EFT_TASK_HIT_2` ..).
- eft_y.c / the head of eft_z.c (`EftLine_*`) and the rest of eft_z.c (`EftBill_*`) are one module (part
  kind 16) under two prefixes: unify.
- `EftShotFx_IsOnScreen` (eft_t_c.c): its `#if 0` attempt does not compile when enabled (parse error).
- `EftBlast_Init` (eft_j.c) matches when compiled with `-fno-gcse` (agent's note): look for a source
  form that defeats gcse there.
- Done at this integration: the `BtlCtrl_*` renames (PlayMotion, StopMotion, IsMotionPlaying, SetRot,
  Transform, Fuse, UseTechnique, SetMaxPower) with their one-to-one `BtlFacade_*` wrappers, the numbered
  stat curves (`BtlStat_GetRate0..3`, `GetScale4..6`, `8..12`), `BtlAct_IsTechniqueId`, and the parameter
  order of `BtlAnim_AdvanceThen` in btl_char_status.h and its definition.
- `BtlAnim_AdvanceThen` is still declared `(chr, next, flags, blend)` in the local externs of
  btl_char_action.c, btl_act_c.c, btl_act_d.c, btl_act_e.c and btl_act_h.h (the registers are the same, so
  it links and matches); converting them means swapping the last two arguments at every call.
- Not renamed: `BtlStat_GetScale7` (second damage-taken multiplier), `BtlFacade_ForceAction01/23/4`,
  `BtlFacade_ForceActions` (they force a reaction on the other side as well, so they are not plain wrappers
  of `BtlCtrl_UseTechnique`), the `SetAuraOn/Off` and Ki / Blast names proposed by the btl_capi_b agent.
  The "ratio" comments in btl_facade.c (the value is a percentage) are not fixed.
- Comments in docs/systems/ still use the old names of the functions renamed here.
- `btl_char_hit.h` describes the attack table as inline at object +0x920; it is a pointer.
- The previous action (fighter +0x950) has no writer anywhere (searched the whole executable);
  see combat.md. Fix the comment-level claims in handlers that assume it works.
- One unified fighter header: every battle file has its own partial view of the 0x1600-byte
  fighter object; the merged picture is in docs/systems/fighter.md and combat.md.
- Names proposed by the script-command agent for `BtlFacade_*` placeholders (lip sync, ki and
  blast gauge adders, CPU level) are not applied yet.
- `Snd_SendFighters` sends sound handles, not fighter ids (name not changed yet). The `ADXF_Tell`
  lock / worker mix-up is fixed (docs/systems/audio.md).
- Functions in linked files that are still INCLUDE_ASM are listed in docs/open_questions.md (seven added
  by the action-handler batch, 62 by the first effect / stage wave, 79 by the second, 13 by the last batch).
- `BtlAi_GetPairRate` / `BtlAi_GetQuadRate` (btl_ai_cond.c): the switch shape that matched
  `AiThink_GetSubRate` may fix them.

## After this batch (docs/roadmap.md)

Effect tasks 0x12DD80..0x1AE200 and the stage 0x23FB20..0x248F28 complete the simulation; then
the headless PC simulation validated against the game's replay format.

## Running now (written 2026-10-04, late)

Last verified and committed state: commit "Link the battle object, collision library, task tree,
text printer and AI scripts" (68.41%, 152 linked C files, 5405 functions clean, no unlinked
decompiled files, working tree clean at that commit).

Six agents were launched after that commit (cap: at most 10 at a time, user request). Their
reports arrive as messages. None may run ninja / configure.py or git; I verify and commit.

| Agent | Edits | Task |
|---|---|---|
| cleanup: battle object | src/battle/bobj_a.c, bobj_b_b.c (+ their headers) | match `BtlObjAnim_SamplePosRot`, `SamplePose`, `BtlObjXf_Update`, `BtlObjAnim_Load`, `BtlObj_BindTables`, `BObjChainB_Build`, `BObjChainB_Step`, `BObjChainA_Step` |
| cleanup: stage | stg_a.c, stg_a_b.c, eft_det_b.c, eft_det_b_b.c | `BtlStage_UpdateObjs`, `BtlStage_BreakObj`, `StgPart_Animate`, `StgFrustum_Build`, `Stg_FadeByCamDist`, `StgCol_SplitStep`, `StgCol_FighterBreakObj`, `StgCol_TraceZone`, `StgNav_FindPath`, `StgNavNode_Clear` |
| cleanup: projectiles | eft_a.c, eft_o_b.c, eft_o_c.c, eft_p.c, eft_i.c, eft_j.c, eft_r.c, eft_h.c | the `EftHit_*` near-misses, blast object, disc, sweep, `EftEmit_Spawn`, `EftBlast_Init`, `EftStruggle_Init`, shot manager |
| cleanup: fighter / AI | btl_ai_cond.c, btl_ai_act.c, btl_ai_seq.c, eft_det_b_c.c, btl_act_a.c, btl_act_c.c, btl_act_h.c, btl_capi_a.c, btl_input.c | six `AiThink_*`, `BtlAi_GetPairRate / QuadRate`, `BtlAiSense_IsBusy`, `BtlAiStep_GuardUntilSafe / Unk17`, `BtlAiSeq_PushRule`, `BtlActB_TickMemberChange`, `BtlAct_GuardHandler`, `BtlAct_GrabDash`, `BtlCharApi_HasKiBlastType2/3`, `BtlInput_Update` |
| vu0_a | NEW: config/symbols/vu0_a.txt, src/port/vu0_a.c (bt3-port repository), include/port/vu0_a.h (bt3-port repository), src/sys/vu0_a_c.c | vector / matrix library 0x11FA10..~0x121000: names, exact portable C reference (`Ref_*`), matching C for the non-VU0 functions |
| vu0_b | NEW: config/symbols/vu0_b.txt, src/port/vu0_b.c (bt3-port repository), include/port/vu0_b.h (bt3-port repository), src/sys/vu0_b_c.c | the same for ~0x121000..0x122940 |

When a cleanup agent reports: rebuild with the gate (configure, ninja exit 0, both .ok files,
both cmp silent), run fdiff over every linked file, then commit. A matched function may have
moved constants from LIT4_WORD / INCLUDE_RODATA lines into C: the ROM compare catches mistakes.
When the vu0 agents report: list vu0_a.txt / vu0_b.txt in both yamls, run
scripts/apply_names.py (all symbol files are listed, so the stock script is safe), link the
matching C files they wrote (src/sys/vu0_*_c.c) if any, rebuild with the gate, write
docs/systems/math.md additions (exact semantics, VU0 vs IEEE differences, R register rule),
commit. src/port/ is not part of the matching build.

Open question put to the user (no answer yet): after the vu0 agents report, have ONE agent
find the equivalent maths routines in the Wii build (wii/, reference only; PowerPC, no VU0)
and compare them with the PS2 reference implementations, as a cross-check of intent. Do not
launch it without a yes.

Also offered, not yet done: extracting PZS3US1.AFS and PZS3US2.AFS (2.7 GB together) from the
ISO into the gitignored disc/DATA/ when asset-format work starts.

Then continue with docs/roadmap.md, "Status 2026-10-04 (evening)", items 2..5.


## Eighth batch: the rest of the main executable (started 2026-10-04, late)

Priority changed by the user: finish the byte-for-byte decompilation before any port work (see
docs/roadmap.md). Brief: docs/briefs_remaining_main.md. Snapshot for agents:
scratchpad/snap3/asm. Both vu0 agents have reported and their files are committed but NOT
linked or listed in the yamls yet (config/symbols/vu0_a.txt, vu0_b.txt; src/sys/vu0_a_c.c,
vu0_a_c_b.c, vu0_a_c_c.c cover 0x11FA10..0x121008 with no gap; src/sys/vu0_b_c.c holds 25
non-contiguous functions of 0x121008..0x122940 and needs INCLUDE_ASM or a split for the gaps).
Link them after the four cleanup agents report (they are editing linked sources).

Launched (10 running with the four cleanup agents):

| Stem | Range | Lead |
|---|---|---|
| src/battle/hud_a | 0x2187E0..0x21CA60 | battle HUD (update 0x218D88) |
| src/battle/hud_b | 0x21CA60..0x222400 | battle HUD continued (one very large function) |
| src/sys/gfxm_a | 0x102F28..0x106D60 | low-level graphics, screen-effect group members |
| src/sys/gfxm_b | 0x106D60..0x10AD58 | depth-to-alpha, blended rectangle, texture upload helpers |
| src/battle/view_a | 0x25C2A8..0x2600B0 | code after the script commands (viewer / demo loop?) |
| src/sys/lib_a | 0x268248..0x26C050 | 516 tiny functions before the CRI library: first establish whether it is library code |

Still to launch (function-boundary cuts): 0x10AD58..0x10EC18, 0x10EC18..0x112A30,
0x112A30..0x115170 (suggested stems gfxm_c, gfxm_d, gfxm_e); 0x115478..0x1198D8,
0x1198D8..0x11EC10 (stage model / draw: stgm_a, stgm_b); 0x2129C8..0x215420 (hud_0);
0x222400..0x226488, 0x226488..0x22A750, 0x22A750..0x22FD10 (hud_c, hud_d, hud_e; the last
includes 0x22F9A8.. pause check and 0x22FC20..0x22FD10 before stg_d.c); 0x252F68..0x254A20
(the second Mersenne Twister / menu codec area: misc_a); 0x2600B0..0x263098 (view_b);
0x26C050..0x26FE90, 0x26FE90..0x273CA0 (lib_b, lib_c: only if lib_a turns out to be game code).
After the main executable: the menu overlay DBZP.BIN (0x7C204 bytes, 737 functions).

## STOP INSTRUCTION (user, 2026-10-04, late)

"when those 10 finish stop for now". Do NOT launch any new agents. For the agents still running
(three cleanup agents: stage, projectiles, fighter / AI; six new-range agents: hud_a, hud_b,
gfxm_a, gfxm_b, view_a, lib_a): when each reports, re-diff its files and record its findings in
docs/. When all cleanup agents are in: run the full gate (configure, ninja, both .ok files,
both cmp silent; a trial build while they were mid-edit did not match, which is expected),
find and fix anything that breaks the image, then commit the cleanup work (bobj_a.c and
bobj_b_b.c from the battle-object cleanup agent are edited and re-diffed but NOT committed
yet). The new-range files stay unlinked and get committed as they are. Then stop and wait for
the user. Not to be done until the user says so: linking the vu0 files, launching the queued
chunks, the menu overlay, any port work.

lib_a reported (2026-10-04): 0x268248..0x269228 is game code (confirmation dialog, 18 of 19
match, committed unlinked); 0x269228..0x273CA0 is the CRI ADX library, so the queued chunks
lib_b / lib_c are NOT needed (naming only, if ever).

## Update (2026-10-04, later): what is left before stopping

User: "you can link it after this subagent finishes then stop for now" (session budget low).
All agents have reported except the fighter / AI cleanup. Committed unlinked: vu0_a / vu0_b,
lib_a (dialog), hud_a*, hud_b, gfxm_a, gfxm_b*, view_a*. Uncommitted in the working tree: the
cleanup edits (bobj_a, bobj_b_b, eft_a / h / i / j / o_b / o_c / p / r, stg_a, stg_a_b,
eft_det_b, eft_det_b_b, stg_d.c / .h prototype fix) and the yaml .lit4 moves for eft_h, stg_a,
stg_a_b. A trial link showed text in place and ONE data shift: btl_act_h now emits 8 more
bytes of .lit4 (the fighter agent matched something), so `[0x1FDEB4, lit4, cod/1FDEB4]` must
go (btl_act_h's pool then runs 0x1FDEB0..0x1FDEBC) once that agent reports; check its report
for other pools. Then: `.venv/bin/python configure.py && ninja`, gate, fdiff sweep, commit,
stop. Linking the new-range files is the next session's first job (one integrator).

## STOPPED HERE (2026-10-04): cleanup linked, build identical, waiting for the user

All agents have finished; none are running. The four cleanup agents' work is linked and
committed: ninja exit 0, both .ok files, both images identical, fdiff sweep clean (the two
"not OK" a grep for "Error" reports are the function names `Movie_CbError` and
`File_AdxErrorCallback`). Main executable 69.83% (was 68.41%); INCLUDE_ASM in linked files
147 (was 185).

Cleanup results: battle object 7 of 8 (both pose samplers match; `BObjChainA_Step` left);
stage 10 of 10; projectiles 9 of 16 (the agent's "10 and 6" headline was miscounted: 7 left,
all checked by a differential interpreter in build/scratch_cleanup_eft/); fighter / AI 12
matched, 7 left (`AiThink_TestSkill`, `AiThink_GetBlastStep`, `BtlAiStep_GuardUntilSafe`,
`BtlAiStep_Unk17`, `BtlAiSense_IsBusy`, `BtlAct_GuardHandler`, `BtlInput_Update`). No
semantic errors found in the old attempts except the one-bit clamp constant in
`BObjChainB_Step` and the `StgRigid_Create(pos, radius, user)` argument order (fixed in
stg_d.c / stg_d.h). The agents' matching lessons are in their reports only (transcript): add
them to docs/decomp_guide.md next session; docs/open_questions.md still lists functions that
now match.

Next session, in order (ask the user before launching anything; keep to 10 agents):
1. One integrator to link the committed-but-unlinked files: vu0_a / vu0_b, lib_a (dialog),
   hud_a* + hud_b, gfxm_a + gfxm_b*, view_a*. Each agent's "for the integrator" notes are
   summarised in docs/systems/{math,graphics,hud,menu_support,audio}.md.
2. The queued chunks of the main executable (list in the "Eighth batch" section above, minus
   lib_b / lib_c which are the CRI library).
3. The menu overlay DBZP.BIN.

## Ninth batch (started 2026-10-04, user said "continue now")

Running: 10 agents (the cap).
- Integrator: linking vu0_a / vu0_b, lib_a (dialog), hud_a* + hud_b, gfxm_a + gfxm_b*, view_a*;
  also refreshing docs/open_questions.md. It edits yamls and linked sources: commit nothing of
  its work until it reports and the gate passes here.
- New ranges (new files only; snapshot scratchpad/snap3/asm; brief docs/briefs_remaining_main.md):
  gfxm_c 0x10AD58..0x10EC18 (movie player), gfxm_d 0x10EC18..0x112A30, gfxm_e
  0x112A30..0x115170, stgm_a 0x115478..0x1198D8 (stage model), mcflow_a 0x1198D8..0x11EC10
  (memory-card dialog flows), hud_0 0x2129C8..0x215420 (battle glue?), hud_c
  0x222400..0x226488, hud_d 0x226488..0x22A750, hud_e 0x22A750..0x22FD10.
Still to launch when slots free: misc_a 0x252F68..0x254A20, view_b 0x2600B0..0x263098; then
the unexplored tail before the SDK (check the yaml for what lies between 0x263098 and the
linked sys files, and 0x2BDCE8..0x2BF488 result-screen code mentioned by the view_a report),
then DBZP.BIN.

## Ninth step: the first new-range files linked (2026-10-04)

Gate passed after each module and at the end: ninja exit 0, both .ok files, both images
identical, `scripts/fdiff.py` clean on all 171 linked files (5877 functions). Main executable
75.62% (was 69.83%); INCLUDE_ASM in linked files 166 (147 + 19 that came with the new files).

- **Vector library: linked.** src/sys/vu0_a_c.c 0x11FA10, vu0_a_c_b.c 0x11FE80, vu0_a_c_c.c
  0x1204B8, vu0_b_c.c 0x121008..0x122940, no gap, no data sections. vu0_b_c.c now holds its 67
  hand-written routines as top-level assembly blocks generated from the split (INCLUDE_ASM does
  not work for VU0 code), so the file is contiguous.
- **Dialog: linked** as src/sys/dialog.c (was lib_a.c; include/sys/dialog.h, with a forwarding
  lib_a.h because src/sys/mcflow_a.c, still being written, includes the old name). Symbols split
  into config/symbols/dialog.txt and cri_adxf.txt; three CRI lock / worker names fixed.
  `.rodata` 0x2F35B0, `.sdata` 0x2FF160. `Dialog_SetCursor` stays INCLUDE_ASM.
- **HUD: linked.** hud_a.c 0x2187E0, hud_a_b.c 0x219EB0, hud_a_c.c 0x21BCA0, hud_a_d.c 0x21C0E0,
  hud_b.c 0x21CA60..0x222400. hud_a_d.c and hud_b.c are one module but were NOT merged: they use
  two views of the gauge work (`HudGauge` in hud_a.h, `HudBWork` in hud_b.c) and the image is
  the same either way; merge when the HUD headers are unified (hud_c / hud_d / hud_e are being
  written against hud_a.h). The health-field correction is applied to hud_a.h.
- **Graphics: linked.** gfxm_a.c 0x102F28, gfxm_b.c 0x106D60, gfxm_b_b.c 0x109938, gfxm_b_c.c
  0x10A6E0..0x10AD58. `.rodata` 0x2EB6A0 (gfxm_b.c), `.lit4` 0x2FC290 (gfxm_b.c) and 0x2FC2B8
  (gfxm_b_c.c), `.sdata` 0x2FE8D8 (gfxm_b_c.c, "LIT").
- **Menu support: linked.** view_a.c 0x25C2A8, view_a_b.c 0x25CFC0, view_a_c.c 0x25D290,
  view_a_d.c 0x25D468, view_a_e.c 0x25DE68..0x2600B0 (view_a_f.c merged into it: one object).

Found only by the image compare, fixed at the source:
1. gfxm_b.c, `GfxAlphaKey_BuildClut`: five of the eleven colours in the 33-byte table were wrong
   (bytes transposed). fdiff cannot see initialiser values.
2. view_a_e.c / view_a_f.c: as two objects the powers-of-ten table sat 8 bytes early; merged.
3. dialog.c: `Dialog_SetCursor` (assembly) refers to two strings of the C part by symbol; they
   are now named objects defined where the string pool has them.
4. `INCLUDE_RODATA` tables (hud_b.c, gfxm_b.c) exist only once the file has a `.rodata`
   subsegment, and a table that a subsegment boundary cuts is split: gfxm_b.c's needed its size
   (0x70) in the symbol file.
Prototype corrections in callers (all re-diffed): `TexFile_UploadOne` (btl_obj.c: uploads, returns
nothing), `StgGlare_Init` / `StgDepthTint_Init` / `GfxDepthFog_Init` / `GfxPost_DrawDepthClut`
(stg_c.c), `HudGauge_ShakeHp / ShakeKi` (hud_a.c), the GS mask callbacks (hud_b.c),
`Num_ToDigits` and `Res_RelocateOffsets` (loading.c, view_a*.c). bobj_a.c still declares
`Res_RelocateOffsets` with two arguments for one call (it matches that way).

Left as it is: asm/ still holds chunks of earlier splits (splat does not delete them); they are
assembled but not linked, and `scripts/progress.py` counts their labels, so its "functions still
in assembly" line is too high. Deleting them was not possible in the integrator's session.


## State 2026-10-04 (after the ninth-step link, commit 966c9dc; 75.62%)

Reported and committed unlinked: mcflow_a (22 / 22), hud_d + hud_d_b (28 / 28), stgm_a
(stage model draw, 13 / 13) + stgm_a_b (memory-card layer `McCard_*`, 20 / 20).
Running (10): gfxm_c, gfxm_d, gfxm_e, hud_0, hud_c, hud_e, misc_a (0x252F68..0x254A20),
view_b (0x2600B0..0x263098), late_a (0x2BD230..0x2BF6B0, the -G0 game code after the SDK),
and the menu-overlay PILOT (chunk 1 of docs/briefs_menu_overlay.md; it must establish how to
verify overlay code with fdiff before chunks 2..27 are launched).
With these every game-code range of the main executable has been attempted. Next: one
integrator for the ninth batch files when most have reported; overlay chunks 2..27 as slots
free up, AFTER the pilot reports its workflow (add its instructions to the overlay brief).
Stale chunks under asm/ from earlier splits inflate progress.py's function count (not the
percentage); deleting them was blocked for the integrator, ask the user before removing.

## State 2026-10-04 (overlay under way)

Main executable reports since the ninth-step link, all committed UNLINKED: mcflow_a, hud_d*,
stgm_a*, hud_c*, misc_a* (password codec), gfxm_d* (clip setters + battle object renderer; VU0
chunk 0x10FFD0..0x1101D0 stays asm), gfxm_e* (model binding, shadows, stage relocation),
gfxm_c (movie player), hud_0* (pause / result menu). Still running in the main executable:
hud_e (0x22A750..0x22FD10), view_b (0x2600B0..0x263098), late_a (0x2BD230..0x2BF6B0).
Overlay: pilot done (chunk 1, 25 / 25, committed; overlay is -G0, fdiff.py and configure.py
now use -G0 for src/menu/); chunks 2..8 running (stems menu_b..menu_h). Chunks 9..27 to
launch as slots free (table in docs/briefs_menu_overlay.md; give each a lead from the mode
table in docs/systems/menu_overlay.md). When the three main-executable agents report: one
integrator for all the unlinked main-executable files (it will take a slot).

## State 2026-10-04 (every main-executable game range attempted)

All main-executable agents have reported (last: view_b*, late_a* = the -G0 wish screen at
0x2BD230..0x2BF6B0). Tenth-step integrator RUNNING: linking gfxm_c / d* / e*, stgm_a*,
mcflow_a, hud_0*, hud_c* / d* / e*, misc_a*, view_b*, late_a* (it may not touch src/menu).
Commit its work only after re-running the gate here; add only tracked changes plus the files
it creates (the overlay agents' new files are committed per report).
Overlay: chunks 1..7 reported and committed (menu_a .. menu_g; 131 functions, 130 match;
`CharSel_Input` 3 instructions off). Running: chunks 8..16 (menu_h .. menu_p). To launch as
slots free: 17..27 (menu_q .. menu_za). fdiff.py uses -G0 for paths containing src/menu/,
src/cri/ or src/sys/late_a. Overlay linking (DBZP.yaml: src_path, per-file .rodata, grouped
.data, G_FLAGS already set) is a later integrator job, after all chunks report.
Overlay facts so far are in docs/systems/menu_overlay.md (mode table, story mode, duel mode,
the versus and team battle hand-offs).

## Tenth step: the rest of the main-executable C files linked (2026-10-04)

Gate passed after each of the eight modules and at the end: ninja exit 0, both .ok files, both
images identical, `scripts/fdiff.py` clean on all 205 linked files (6420 functions). Main
executable 86.91% (was 75.62%); INCLUDE_ASM in linked files 174 (166 + 8 that came with the new
files: `Flash_Advance`, `ObjShadow_BuildPacket`, `BtlText_PutSprite`, `HudPrompt_UpdateCue`,
`TextBox_DrawClip`, `ShenScene_StepSeq`, `Shen_DrawList`, `Shen_BuildList`; all in
docs/open_questions.md). Nothing was backed out. Every game-code range of the main executable
that has C is now linked; what is left in assembly there is the VU0 chunk 0x10FFD0..0x1101D0,
0x240C68..0x240DB8 (`StgVu_Rotate*`), the middleware / SDK (0x269228..0x2BD230 except adxt.c) and
the INCLUDE_ASM functions.

- **Movie player**: gfxm_c.c 0x10AD58, gfxm_d.c 0x10EC18..0x10FB40. Three objects with gfxm_b_c.c
  (not merged: same image, and the `Flash*` / `FlashD*` headers and the local views in dialog.c /
  view_a*.c stay apart). `.rodata` 0x2EB740, `.sdata` 0x2FE8E0.
- **Battle object renderer**: gfxm_d_b.c 0x10FB40, `asm` 0x10FFD0, gfxm_d_c.c 0x1101D0, gfxm_e.c
  0x112A30, gfxm_e_b.c 0x1131D8, gfxm_e_c.c 0x114860, gfxm_e_d.c 0x114B18..0x115170.
- **Stage model draw + memory card**: stgm_a.c 0x115478, stgm_a_b.c 0x116B98, mcflow_a.c
  0x1198D8..0x11EC10. `gStgAnimTable` is now defined in stg_d_b.c, `gMcCardStep` / `gMcCardCmd`
  in stgm_a_b.c, `gMcFlow` in mcflow_a.c (all `.sdata`).
- **Pause menu**: hud_0.c 0x2129C8, hud_0_b.c 0x213238, hud_0_c.c 0x215010..0x215420.
- **HUD rest**: hud_c.c 0x222400, hud_c_b.c 0x222730, hud_c_c.c 0x224B50, hud_d.c 0x226488,
  hud_d_b.c 0x226500, hud_e.c 0x22A750, hud_e_b.c 0x22B4F8, hud_e_c.c 0x22EC08, hud_e_d.c
  0x22F998, hud_e_e.c 0x22FC40..0x22FD10. Ten objects (not merged: the four `.sdata` work
  pointers land at 0x2FEB58 / 5C / 60 / 68 either way); `gHudCombo` is defined in hud_c_b.c.
- **Password codec**: misc_a.c 0x252F68, misc_a_b.c 0x253ED8..0x254A20.
- **Menu support rest**: view_b.c 0x2600B0, view_b_b.c 0x260D20, view_b_c.c 0x2614B0, view_b_d.c
  0x261ED8, view_b_e.c 0x262FF0..0x263098 (view_b.c not appended to view_a_e.c: same image).
- **Wish screen** (-G0): late_a.c 0x2BD230, late_a_b.c 0x2BEB20, late_a_c.c 0x2BF370..0x2BF6B0,
  split off the end of the big library chunk; `.rodata` 0x2FBDA8 / 0x2FC0F0, `.data` 0x2EB34C /
  0x2EB350. configure.py's `G_FLAGS` keys are path prefixes now (`"src/sys/late_a"`).
Data placement of each file: the "Linked layout" line of its section in docs/systems/.

Link-time problems (none visible to fdiff), fixed at the source:
1. gfxm_c.c called the 14 clip-list functions of gfxm_d.c by names of its own (`FlashClips_*`);
   renamed to gfxm_d's `FlashClipList_*` (mapping taken from the original call targets).
2. mcflow_a.c had private `#define` aliases for the card layer. Applying the listed names turned
   them into macros over real names (`#define McCard_GetInfo McCard_Probe`), so the wrong
   functions were called and it no longer compiled. The aliases are removed and the calls use the
   names of config/symbols/stgm_a.txt (checked relocation by relocation against the image).
3. hud_e_b.c: its `.rodata` ends at 0x2F2164 and the next object's table (col_a) is at 0x2F2170;
   an 8-byte assembly chunk `[0x1F2168, rodata]` carries the original's 16-byte padding.
4. Password tables: the assembly `.sdata` chunk behind them starts 4 bytes early (0x1FF05C).
5. view_b_d.c begins at 0x261ED8 with `ShenScene_StepSeq` (INCLUDE_ASM under `#else`), not at
   0x262890; its `.rodata` (four INCLUDE_RODATA tables, then the jump table) needed
   `RODATA_ALIGN16()` in front of the first table, and the object also owns the six words at
   0x2F3470.
6. late_a.c: with a `.rodata` subsegment splat moved the two strings `Shen_DrawList` uses into
   that function's .s file, where the C already defines them: `force_not_migration:True` on
   `gShenIconClip` / `gShenTextClip` in config/symbols/late_a.txt.
7. late_a*.c refer to overlay symbols from C (`D_003B0EB4`, `func_00399240`, `func_00399430`,
   `func_00399478`, `func_00399730`, `func_00399760`); splat only writes undefined-symbol
   entries for assembly references, so they are defined in config/linker_script_extra.ld. Give
   them the overlay's names when config/symbols/menu_*.txt are listed.
8. late_a_c.c's `.data` ends at 0x2EB354: the `.eh_frame` chunk starts at 0x1EB354 (4 bytes of
   padding first).
9. A stale-object trap: objects do not depend on asm/nonmatchings/*.s, so after a re-split that
   changes one (a jump table moving into a function's .s) the object must be deleted by hand.
Comment fix: the character grid markers were swapped. 0xA1 is the random cell and 0xA3 the saved
custom characters (`CHRGRID_ID_RANDOM` / `CHRGRID_ID_CUSTOM` and the two `CHRGRID_NO_*` flags in
view_a_e.c exchanged names; the code is unchanged and re-diffed).

Helper scripts of this step: build/scratch_integ10/ (`go.sh` = configure + gate, `pre.sh` =
configure, apply listed names, compile, locate data, check relocations; `rc.py` compares every
`.text` relocation of an object with what the original image refers to at that place and prints
names that are unlisted or point elsewhere; `od.py` = objdata.py that resolves jump tables;
`starts.py`, `conf.py`, `yed.py`, `ren.py`, `an.py`).

Left as it is: stale chunks of earlier splits under asm/ (more of them now; assembled, not
linked; progress.py's "functions still in assembly" counts their labels). include/sys/lib_a.h
(the forwarding header) has no user left and can be deleted. The menu overlay is untouched
(nothing under src/menu/, include/menu/ or config/symbols/menu_*.txt was read into the build).

## State 2026-10-04 (tenth step linked; all overlay chunks launched)

Main executable: 86.91% linked and verified here (commit ee4fe98); 174 INCLUDE_ASM in linked
files (rows in docs/open_questions.md). A stray compiler dump `t.c.09.loop` sits untracked in
the repo root (an agent's; safe to delete).
Overlay: chunks 1..19 reported and committed, UNLINKED (menu_a .. menu_s): 449 functions, 446
match as delivered (`CharSel_Input` 3 instructions off; `Train_BuildLists`; `Train_Update` /
`Train_Input` match only merged with menu_h_d.c). Running: chunks 20..27 (menu_t .. menu_za).
No more chunks to launch. When they have all reported: ONE overlay integrator (DBZP.yaml
`src_path` is src/dbzp but sources are in src/menu; per-file `.rodata`, grouped `.data` work
pointers that need `= NULL` definitions; the many merges listed per chunk in
docs/systems/menu_overlay.md and in each file's header comment; unify the duplicated views
(TeamSel, HistSel, Train, SimDay, MisSel, UbScore, save views flat vs nested); `Snd_PlaySe`
returns s32 (menu_a.h is wrong); 0x31EA80..0x31EAA4 overlap; overlay symbols referenced from
late_a*.c via config/linker_script_extra.ld). Consider two integrators in sequence (first
half / second half of the overlay) to keep each one's context manageable.
After that: near-miss cleanup rounds on the remaining INCLUDE_ASM functions of both binaries.
Open port questions recorded so far (not to be worked on until the user says): music id 0x18
("random") passed unresolved to the battle by sim / course / mission / survival hand-offs;
Disc Fusion needs a replacement on PC; pause, pause-menu result and CPU-level writes reach
the simulation outside the pad stream; menus and HUD advance shared random generators.

## HOLD (user, 2026-10-04): "dont add more agents till session resets"

Launch NO new agents (no integrator, no cleanup) until the user says the session / usage has
reset. The eight overlay agents already running (chunks 20..27) may finish; process their
reports as usual (re-diff, document, commit) and then wait.

## STOPPED (2026-10-04): every overlay chunk has reported; no agents running; credits low

Overlay: all 27 chunks committed UNLINKED (src/menu/menu_a.c .. menu_za_e.c, 93 files). 737
functions: 723 print OK as delivered; 12 are INCLUDE_ASM with attempts (`CharSel_Input`,
`Train_BuildLists`, `SimEv28` (assembler delay-slot rule, C is right), `SimEv34`,
`Option_Draw`, `DcList_Draw`, `Dc_Main`, `DcMenu_DrawPlates`, `DcPass_WrapPos`,
`DcPass_Decode`, `DcPass_DrawStatus`, `DcPass_DrawRows`); `Train_Update` / `Train_Input`
match only merged with menu_h_d.c. Chunks 25, 26 and the menu_za.c part of 27 were wrapped up
early: no relocated byte compare on menu_y.c and menu_z*.c.
Main executable: 86.91% linked (ee4fe98).
Next, when the user says the session has reset (HOLD above still applies until then):
1. Overlay integrator(s): notes in the previous "State" section plus: `li.d` prelude rule,
   `litodp` / `dptoli` aliases, the `SimEv28` assembler rule, `ItemHelp_*` renames in
   config/linker_script_extra.ld.
2. Near-miss cleanup rounds on both binaries.
docs/systems/menu_overlay.md has every mode's flow and every battle hand-off, and the replay
start / save path.

## Resumed 2026-10-05 (user: "ok continue its been reset"): HOLD lifted

Decision recorded 2026-10-04: the decomp phase ends when all game code of both binaries is
linked, SDK and CRI libraries excluded. Port and decomp will be separate repos (recommended,
port forked from a tagged decomp commit). Now running: overlay integrator (first pass).

## Eleventh step: linking the menu overlay (2026-10-05, in progress)

State file for a second integrator; updated after every group. Tools: build/scratch_integ11/ (`ov.py <stem>` =
relocated byte compare of one file's `.text` / `.rodata` / `.data` against DBZP.BIN; `linked.txt` = stems linked;
`mk.py` rewrites the subsegments of config/DBZP.yaml from it; `go.sh` = configure + gate; `raw.py` = fallback .s
for INCLUDE_ASM; MERGE_BRIEF.md = what the merge agents were told).
Infrastructure done: DBZP target reads sources from src/menu (`src_path: src`, subsegments `menu/<file>`,
`nonmatchings_path: ../nonmatchings`); every config/symbols/menu_*.txt listed in both yamls and names applied;
configure.py writes `build/<target>_extern_syms.ld` (listed symbols outside the image) for both targets;
config/linker_script_extra_dbzp.ld (`litodp` / `dptoli`); `li.d` in the prelude; `Snd_PlaySe` is `s32` in
menu_a.h; `.rodata` subsegments carry `linker_section_order: .data` so data and read-only data interleave in
yaml order; menu_t_b.c is assembled with -G8 (`AS_G_FLAGS`), `SimEv28` is C now.
Merges were done by eight parallel agents (one per run of chunks) and verified with `ov.py`; tails deleted.

**DONE: the whole overlay is linked.** 0x334C00..0x3B0E04 from 69 C files (table of merged objects in
docs/systems/menu_overlay.md "Link state"); build/DBZP.BIN and build/SLUS_216.78.rom byte-identical; the only
non-C subsegment left in config/DBZP.yaml is the zero padding `[0x07C204, data]` (0x3B0E04..0x3B0E80).
`python3 scripts/fdiff.py` prints OK for all 737 functions over the 69 files.
- Newly matching through the merges / toolchain: `CharSel_Input`, `Train_Update`, `Train_Input`, `DcList_Draw`,
  `SimEv28`. INCLUDE_ASM left in the overlay (9; rows in docs/open_questions.md): `Train_BuildLists`, `SimEv34`,
  `Option_Draw`, `Dc_Main`, `DcMenu_DrawPlates`, `DcPass_WrapPos`, `DcPass_Decode`, `DcPass_DrawStatus`,
  `DcPass_DrawRows`. Main executable unchanged: 174 INCLUDE_ASM, 86.91%.
- scripts/progress.py: DBZP 0x78990 / 0x7C204 bytes in C (97.16%); it now reads only the assembly files the
  yaml names (old split files stay under asm/).
- Link-time fixes: menu_z_c.c (`DcMenu_Init` lacked its twelve "host:" path strings; two inline helpers moved
  behind `DcMenu_DrawPlates` for the string order; section fix after the INCLUDE_ASM); EvoTop / DcList got their
  host path strings and helper order from the merge agent; menu_u_g / menu_u_h are two objects; `gBracket` is
  defined in menu_k_b.c; named shared strings for the INCLUDE_ASM functions (`sSimPopoX`, `sSimPopoClip`,
  `sDcMenuPlateClip`, `sDcPassPlate2Clip`, `sOptOnStart`, `sOptClip*`).
- Not done (optional clean-up): the remaining duplicated struct views across include/menu/*.h (e.g. `Bracket`
  in menu_k.h vs `LBracket` in menu_l.h, `SimDay` / `TSimDay` / `USimDay`, `EvoZ` / `UEvoZ`, flat vs nested save
  views, `ubFlags` vs `discFlags` for gProgress + 0x684); stale file names in comments of config/symbols/menu_*.txt
  and a few sources; `MSave` should move to include/sys/save.h.
Next: near-miss cleanup rounds on the INCLUDE_ASM functions of both binaries.

## Twelfth step started 2026-10-05: near-miss cleanup round 2

State: BOTH binaries fully linked and byte-identical, verified here after a fresh configure +
ninja (commit da1c694): main executable 86.91% (174 INCLUDE_ASM), overlay DBZP.BIN 97.16%
(9 INCLUDE_ASM; 69 files in src/menu). This is the stopping point the user named for the
decomp phase ("whole game linked other than the sdks"); the cleanup below was the planned
next step.
Running: 9 cleanup agents (brief docs/briefs_cleanup.md; tags A..I; file lists in the launch
prompts: A eft_a..e, B eft_g..j, C eft_n..s, D eft_t_b..y, E eft_z..ab_c, F eft_ad_b..ae +
stage + misc, G src/sys graphics, H fighter / AI / HUD / menu support / late_a, I overlay).
They edit LINKED files and report data-pool changes. When each reports: re-diff its files,
apply the yaml changes it lists (.lit4 / .rodata moves), then `.venv/bin/python configure.py`
+ ninja gate; the gate is only conclusive when no agent is mid-edit, so commit after all (or
after isolating one agent's files). Not yet done / user has not answered: setting up
decomp-permuter for the register-only near-misses.

## Twelfth step done 2026-10-05: cleanup round 2 linked

All nine cleanup agents reported; gate passed here after a fresh configure + ninja (both
images identical); fdiff sweep over every linked file of both binaries: 7157 functions OK, 0
not OK. Main executable 91.48% (was 86.91%), 97 INCLUDE_ASM (was 174); overlay 99.02%, 6
INCLUDE_ASM. 80 functions matched this round (A 16, B 10, C 14, D 9, E 3, F 14, G 8, H 3,
I 3). One link-time error: eft_e.c called `EftUtil_IsCamBelowLevel`, a name that never
existed (real: `EftUtil_IsCamUnderWater`, 0x1473E8); it had been hidden inside `#if 0`.
Fake matches (byte-identical through a stand-in, marked in the sources and the guide):
`EftLink_DrawBillboard` (empty `__asm__("")`), `EftBlast_Init` (volatile read),
`EftChain_DrawStrand` (volatile alias of gOtCur), `Dialog_SetCursor` (invented dead branch);
`EftWater_DrawSprayQuad` uses a backward goto.
One behavioural error found and fixed in an old attempt: `EftGlow_SpawnPart` colour step.
Yaml .lit4 moves applied: eft_b (0x1FC370), eft_g (0x1FC708..0x1FC728), eft_ad_b / eft_ad_c
(0x1FCEEC / 0x1FCEF4), btl_char_fx (chunk 0x1FD200 removed), bobj_b_b (chunk 0x1FE718
removed).
TODO: docs/open_questions.md rows are stale for the 80 matched functions (rebuild from the
INCLUDE_ASM lines); `ColObb_Contact` needs a from-scratch decompile (dead code, 17 KB);
`EftGfx_DrawSprite` and `StgBlur_Draw` have no attempt; the EftPart10 / EftLink key functions
need nested track structs. Next (user agreed): set up decomp-permuter for the register-only
near-misses.

## Thirteenth step 2026-10-05 (afternoon): permuter + targeted agents

State: main executable 91.97% (85 INCLUDE_ASM), overlay 99.11% (5), both byte-identical
(commit 87a91a2). decomp-permuter is set up (scripts/permute.py, build/permuter/); batch 1
gave 13 matches. Ten functions are fake matches (six tagged `FAKE MATCH` in the source; four
older ones to tag the same way: `EftLink_DrawBillboard`, `EftChain_DrawStrand`,
`Dialog_SetCursor`, `EftWater_DrawSprayQuad`; `EftBlast_Init` is no longer one).
Running:
- permuter batch 2 (27 functions, build/permuter/batch2.txt, results.txt; ends about 18:15;
  a background watcher reports). Its files must not be edited meanwhile: late_a, hud_0_c,
  eft_ab_c, eft_p_b, view_a_e, eft_aa, menu_z_d, hud_e_b, eft_b, eft_z, gfx_ot, gfxm_c,
  sprite, eft_c, btl_demo_cam, eft_i, eft_w.
- five agents on disjoint files: K (key-track struct redesign: eft_x.c, eft_v_c.c),
  S (simulation-critical: btl_input.c, btl_ai_seq.c, btl_ai_cond.c, with a differential
  test harness first), P (`EftGfx_DrawSprite` from scratch, eft_a.c), compiler-version /
  flag investigation (no source edits; build/scratch_compiler/), V (screen passes:
  stg_c.c, stg_b.c, gfxm_a.c, gfxm_b.c, gfxm_e_b.c).
When they report: apply data moves, gate, commit. TODO after: tag the four older fakes;
rebuild docs/open_questions.md from the INCLUDE_ASM lines; `ColObb_Contact` is the only
function with no faithful attempt (dead code, 17 KB; user has not asked for it).

## Fourteenth step 2026-10-05 (late afternoon): cleanup round 3 + behaviour test sweep

State: main executable 93.28% (70 INCLUDE_ASM), overlay 99.11% (5), byte-identical at commit
6d1fed5. NOTE: commits 8f74a33 and 90e054b do not build (a symbol comment splat rejects; a
dropped line in configure.py); each is fixed by the commit after it. RULE: run the gate
before EVERY commit that touches anything outside docs/, however trivial.
Toolchain check done: we use the game's compiler and flags (docs/decomp_guide.md); the
stubborn families are source-form problems.
Running: permuter batch 2 (watcher bt423p9w0; files not to edit listed in the thirteenth
step); agents W1 (eft_g, eft_n, eft_q, eft_s), W2 (eft_e, eft_d_b, eft_t_b, eft_u_b,
eft_ae), X1 (ChrCam_CalcCut first, btl_scene, btl_seq, hud_b, view_b, view_b_d, gfxm_a,
gfxm_b, gfxm_e_b), X2 (the two open families: eft_ab, eft_z_b, eft_z_c, eft_h), and a
differential behaviour test sweep over every unmatched function (no source edits; results
in build/scratch_difftest/results.md).
After they report: apply data moves, gate, commit; apply permuter batch 2; tag the four
older fakes with `FAKE MATCH`; rebuild docs/open_questions.md. Then the user decides:
continue matching rounds, or tag the decomp and start the port repo (recommended once the
behaviour sweep is clean).

## HANDOFF NOTE (2026-10-05, about 16:30) -- read this first after a compaction

LAST GOOD COMMIT: 6d1fed5 (+ 0f2f830 docs) = main executable 93.28%, overlay 99.11%, both
byte-identical. The WORKING TREE has UNCOMMITTED, individually fdiff-checked work from round 3:
- X2 (families): 7 matched in eft_ab.c, eft_z_b.c, eft_z_c.c, eft_h.c (+ include/battle/eft_ab.h
  `uv` is now `f32 uv[16][4]`).
- W2: `EftPtcl_DrawAxisQuads` live in eft_u_b.c (FAKE: volatile alias gOtCurRead); improved
  attempts in eft_e.c, eft_t_b.c, eft_ae.c.
- X1: `ChrCam_CalcCut` (btl_char_cam_cut.c), `BtlText_DrawScrollBar` (btl_seq.c; old attempt
  had a BEHAVIOURAL ERROR: both thumb parts are always drawn), `StgPanBlur_UpdateView`
  (gfxm_a.c; FAKE: `Vec4 *sp = &side;` and a `do { } while (0)`); notes only in btl_scene.c,
  hud_b.c, view_b.c, view_b_d.c.
- yaml .lit4 moves ALREADY APPLIED in the working tree: eft_z_b -> [0x1FCDCC, .lit4],
  eft_z_c -> [0x1FCE28, .lit4] (their asm chunks removed), [0x1FD08C, .lit4,
  battle/btl_char_cam_cut], [0x1FC280, .lit4, sys/gfxm_a].
THE GATE CURRENTLY FAILS for ONE known reason: build/src/battle/eft_g.o now emits 4 bytes of
`.sdata` (at 0x2FE9DC, shifting gEftSteam etc. by 4): agent W1 (still running; files eft_g.c,
eft_n.c, eft_q.c, eft_s.c) has made a function live there. When W1 reports, apply ITS data
moves (expect a `.sdata` subsegment for battle/eft_g at 0x1FE9DC.. cut out of cod/1FE9DC, plus
whatever it lists), then `.venv/bin/python configure.py && ninja`, both .ok files, both cmp
silent, and ONLY THEN commit everything (`git add -A config src include docs`). If the image
still differs: compare the link map's symbol addresses with their `D_/func_` names to find the
first shift (python snippet used all day), fix, repeat.
Also running:
- permuter batch 2 (build/permuter/results.txt; watcher task bt423p9w0; ends ~17:00). When it
  ends: launch ONE agent to apply the exact solutions like batch 1 (brief = the "Apply
  permuter solutions" prompt: port diff, split it, look for the natural form, mark fakes,
  try siblings; solved so far: BtlText_PutSprite, BgmList_ApplyUnlocks, EftRibbon_DrawKind1,
  EftAnimPart_Draw, DcPass_DrawRows, EftRibbon_PlaceStrip (a "surviving copy" member: carry
  its form to EftRibbon_PlaceTrail2 and EftBlade_GetColor); Num_DrawEx is at score 10).
- differential behaviour test sweep over all unmatched functions (no source edits; results in
  build/scratch_difftest/results.md): summarise for the user; any DIFFERENT is a bug to fix.
Sunshine (the user's streaming host) runs as background task buxegqcs7 of this session.
Then: tag the older fakes with `FAKE MATCH` (EftLink_DrawBillboard, EftChain_DrawStrand,
Dialog_SetCursor, EftWater_DrawSprayQuad, EftPtcl_DrawAxisQuads, StgPanBlur_UpdateView,
BtlText_DrawScrollBar's two-step form), rebuild docs/open_questions.md from the INCLUDE_ASM
lines, fix prototypes flagged wrong (EftGndDust_SpawnPieceEx in include/battle/eft_z.h: floats
first; stale note in btl_char_cam.c), add this round's lessons to docs/decomp_guide.md (they
are only in the agents' reports in the transcript: X2, W2, X1). Then ask the user: continue
matching rounds, or tag the decomp and start the separate port repo (my recommendation once
the behaviour sweep is clean; permuter keeps running in the background).
USER RULES: at most 10 agents; local commits only, no Claude co-author line; gate before
EVERY commit outside docs/; document findings; the user prefers loose folders over AFS for
the port; decomp phase ends at "all game code linked, SDK excluded" (reached).

## UPDATE to the handoff note (2026-10-05, about 16:50)

Round 3 is COMMITTED and verified: c0d4955 = main executable 94.48% (55 INCLUDE_ASM), overlay
99.11% (5), both byte-identical. The uncommitted-work section of the handoff note above no
longer applies (W1 reported; eft_g / eft_q `.sdata` words placed). All four matching agents of
round 3 are finished; lessons are in docs/decomp_guide.md. Still running: permuter batch 2
(apply its exact solutions with one agent when it ends) and the differential behaviour test
sweep. Remaining follow-ups unchanged: tag older fakes, rebuild docs/open_questions.md, fix
flagged prototypes (EftGndDust_SpawnPieceEx in eft_z.h; Vu0Cur_ProjectPoint declared void in
eft_p_b.c: retry `EftGlow_DrawParts` with the right type), then the user's decision
(more matching vs tag + port repo).

## HANDOFF NOTE 2 (2026-10-05 evening) -- supersedes the earlier handoff notes

VERIFIED STATE: commit f01ce65 = main executable 94.92% (42 INCLUDE_ASM), overlay 99.24% (4),
both byte-identical; working tree clean. ~7,270 functions match; about 21 are marked
`FAKE MATCH` in the sources (list in docs/decomp_guide.md sections of 2026-10-05).
Behaviour test sweep DONE (docs/decomp_guide.md, build/scratch_difftest/results.md): every
unmatched function except `ColObb_Contact` is IDENTICAL to the original; the one difference
(`EftChain_BlendKeys`, an original copy-paste bug our C had "fixed") is corrected and in
docs/known_bugs.md. Toolchain check DONE: compiler and flags are the game's.
RUNNING:
- agent a23584d35485ba2aa: `BtlInput_Update` only (src/battle/btl_input.c; last unmatched
  function of the fight core; 4 of 149; either matches it or writes a rigorous equivalence
  argument). When it reports: if live, apply any data move (its LIT4_WORD D_002FD294), gate,
  commit.
- permuter batch 3 (build/permuter/batch3.txt, 34 functions, results.txt, watcher task
  b0tv6y1fu; ends about 21:30): `EftStreak_DrawScreen` solved so far. When it ends: one agent
  to apply exact solutions (brief = the "Apply permuter batch 2" prompt in the transcript:
  verify.py EVERY candidate first (a score of 0 is not proof), split the diff, natural form
  or FAKE MATCH tag, try siblings, report data moves), then gate and commit.
- Sunshine (user's streaming host) = background task buxegqcs7.
TODO afterwards: rebuild docs/open_questions.md from the INCLUDE_ASM lines (stale); fix
`EftGndDust_SpawnPieceEx` prototype in include/battle/eft_z.h (floats first) and drop the
aliased local declarations; stale note in btl_char_cam.c; `EftRibbon_DrawKind1` permuter
candidate was WRONG (not applied). Long targeted permuter runs only for functions that moved
(`EftGndDust_SpawnBodyDust` 12 of 347, `DemoCam_Update`, the geysers).
DECISION PENDING WITH THE USER: keep matching, or tag the decomp and start the separate port
repo (my recommendation: move on; the permuter can keep running). Port preferences recorded
in memory: loose folders instead of AFS, separate repo forked from a tagged decomp commit,
interpolated rendering over an untouched 30 Hz simulation (discussed, not formally decided),
SDL3 GPU suggested for the renderer (not decided).
RULES: at most 10 agents; local commits, no Claude co-author line; run the gate before EVERY
commit that touches anything outside docs/ (two slips today: 8f74a33 and 90e054b do not
build, each fixed by the next commit); document findings.

UPDATE to handoff note 2 (2026-10-05, later): `BtlInput_Update` is DONE (commit 674709e, FAKE
MATCH via an empty asm with operands, behaviour-tested on 4000 seeds). Main executable 94.96%
(41 INCLUDE_ASM), overlay 99.24% (4); byte-identical. The fight simulation's core has no
function left in assembly. Only the permuter's batch 3 is still running; no agents.

## 2026-10-06: the decomp holds PS2 code only

All PC-port material was taken out of this repository: the 82 `#ifdef PORT` / `#ifndef PORT` blocks in 34 game
source files (the PS2 branch was kept) and the PC reference vector library (`src/port/`, `include/port/`). They
live on in the port repository (`bt3-port`), which has its own copy of the game sources and now carries its
PC-only changes there. Rule from here on: nothing PC-specific is added to this repository. Gate after the removal:
both images match.
