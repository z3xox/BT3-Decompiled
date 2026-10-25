# Wrong or stale names and comments found while naming placeholders (2026-10-08)

## Resolved in names-pass-3 (2026-10-25)

- AI: the +0xD74 and +0x974 getter comments, the health locals of steps 18/19/21 (lowHp / highHp) and `reached` of steps 15/23 were already right.
- AI: the tables indexed by animation id are animClass / animFlags (BtlAiActTable: classByAnim, since animClass changes BtlAiStep_Unk23), startAnim, AiActAtk.anim; "state id" comments say animation id.
- AI: BtlAiWork.costMin / costMax, BtlAiPlan.rolls[].acc, BtlAiOutput.accX / accY and the act10Dist words are f32, BtlAiStatus.hitPendingTimer.
- AI: the AI arrays of group 02 were already split; docs/systems/ai.md describes the four decompiled files.
- Object +0xCAD is hitNo in the btl_char_api_2.h views; BtlChange_RequestChara / BtlChange_GetArgs / BtlChangeReq call the second animation file anim1Chara.
- Effects: BtlStage_GetFxCount0..2 / GetFxList0..2 (counts s32), BtlParam_GetKiRecoverGoal, BtlSkill_GetShotScale, BtlSuper_GetShotScale, BtlSkill_GetClashPower, EftRibbon_SetNode, EftShotFx_Init.
- Effects: 0x2EC2E0 is gVu0AxisYW (vu0 constants; gVu0OneVec 0x2EC2B0 and gVu0AxisXW 0x2EC2D0 declared); every class table is void *[6]; the EftLine_* helpers of gEftBillClass keep their prefix, the comments say so.
- Effects: texture sets: `ready` mask, entry +8 `def`; definition +0x04 / +0x05 kind / sub in every view; +0x18 of the emitter definition is `life` (and the spawn helpers' parameter).
- Effects: part 10 / 9 pulse fields mul*, EftPart10.emitLife, dist, sizeMul, vel / accel, COLOR_RAMP; EftUPtclDef angX..spinZRange; EftLinkNode dir / origin / rotZ; EftKDef.level; EftJFlashArg / EftFlashArg hold f32.
- Effects: the launch block views take EftRArg's names (FxHitArg2 srcId / objId / kind / level, EftHitAtk.node, EftDiscAtk.texVariant).
- Prototypes: EftGndDust_SpawnPieceEx (eft_ground_dust.h) agrees with the definition and replaces the asm-label aliases; the EftEmit_SpawnOwn externs use the definition's parameter names.
- Already settled by earlier passes: the rush-sequence frame comment, the EftJPart / EftJSrc / EftJDef views, the hand comment, the zap type byte, EftDetStageCtx.delta, BtlKiBlastData.radius, the aura spark views (comment fixed), BtlScriptCmdWindow.shadowColor (unk30 commented).
- Effects: EftSkillSrc / EftSuperSrc flagsA + flags as in btl_tech.h; EftHTask cls / prev / next / nextFree; EftVolleyShot script / speed / maxTurn; EftBlastObjArg.nodeSlot; the geyser locals' accel.
- Effects: EftLinkMgr puts node[] at 0x10 and `next` at 0x10690 like EftWLinkMgr (eft_link_2.c indexes it there; nothing reads EftLinkMgr.node).
- Already settled: EftEmitRayArg, EftSetHead widths, EftTTaskList.head, the EftTTask comments, EftShotParam.sub comment, EftGndDustStage, spinB / hold, EftWLinkNode, the EftEmitArg texture blocks, EftSurfTex / EftBurstTex, specular, EftTechArg.chr, the zap work names, the obj_gs_env.h words, StgView scissorY0 (stg.c:371 / 382 compare scissorY0 - scissorX0: the original's own code).
- Fighter: BtlAnim_SetScale (fighter +0x988); BtlCharApi_GetPlayerCharPack returns s32 * like its caller declares it; 61 of the 66 cast macros of fighter functions are gone (the 5 left change the return type to a local object view, or the code).
- Fighter: accessors after their fields: BtlParam_GetTransSeq / GetTransKind / GetFusionCost, BtlOpp_GetSizeClass / GetCharaFlags, BtlCharApi_GetSizeClass / GetRushFinisher / GetFinisherChoice0 / 3 / GetComboFinish / HasCharaFlag80.
- Fighter: comments fixed or already right: GetTechniqueFramesLeft, the jaw tracks, technique 0x280 (six fixed characters by the frame counter), rule word 0x10, the spread local, GetOppTechniqueKind / GetOppSkillKind, mix, BtlObjMdl.anims; BobjFace.blinkFrames, BobjParam charaFlags / sizeClass / auraKind.
- Fighter views of groups 03, 04, 05 (BtlCtlNode, BtlCollReact, BtlFlagPose, BtlActVitals, the camera words, player / side, padFlagA, the prev* placeholders, switchPrompt, the competing names): settled by the merge into BtlFighter (docs/names/fighter/canonical.tsv).
- Progress / save: gProgress +0x3C is `difficulty` in every view (HistProgress.level renamed); the stale gSaveData->unkA08 / unkA0C mentions say tourFlags / tourHour (tour_menu.c, bracket.h, tour_entry.h, menu_overlay.md); the menu save views split unk778[2] into unk778 / ladderRank; SaveRec.item is u16[8] + unk10.
- docs/systems/save_data.md: the layout table follows include/sys/save.h (camDistMode / camShakeOff, the tournament, mission, course and shop words) and the file names of the card and password sections are the current ones; menu_overlay.md names +0x1694 / +0x1698.
- Types: McCardInfo.buf and BObjCommon.boot are pointers, HudPromptObj.skillText is u16 * (with BtlMenu_SetScript2's real parameter), EftHStartArg.scale f32, the group `parts` words void *, EftWaterView.tex[].image a pointer; EftArg12.type stays u8[1] (a plain u8 changes the initialiser's code).
- Already settled by earlier passes: the System / menu support items (adx voice slots, padBit, the gfx_screen extern, the rigid literal, TextBox, SetMaxSize, the memcard and vu1_packet headers), the battle_setup parameters and menu comments, UbScore.line, McFlowReq (commented), TourInfo (commented), 0x640 cursor / 0x684 ubFlags, save ubFlags / rank / missionPages, the slot / baseLevel comments, EvoZ bg, dragonBallFound, the MsgWin_Init blocks (commented), the unk locals, EftWLinkArg.texIdxA, EftKCamArg.anim, skillText, players, replaySlot[7], the zap texIdx / type. eft_burst.c's second view is spelled EftViewE.
- Group 20 function names: the BtlStage_GetFx*, shot scale, clash power and EftRibbon_SetNode renames are done; BtlParam_GetUnkC7Scale stays (what +0xC7 scales is not known).
- Already settled: BattleRule.dragonBall / BattleResult.dragonBallFound / BattleRes.hudFile (its comment says gProgress->language now), EftAuraCfg.flame[] (commented in place), EftAuraCfgFlame partNode / nodeEnd / nodeRef.

## Resolved by the fighter merge (2026-10-25)

The fighter is one structure now (include/battle/btl_fighter.h; every field with the old names in
docs/names/fighter/canonical.tsv). Items of this file that it settled and that are removed below: the
names of +0xD4C / +0xD50 (combo.newHit / combo.changed), +0xE60 (boostStock, 0..10), +0x964 / +0x968
(actionFrame / prevActionFrame), +0xE44 (techCharge), +0xEFC (stepFrame[5]), +0x1288 (actBits),
+0x1294 (framesLeftOverride), +0x1310 (bodyWarpPos), camera +0x460 (cam.bodyPos), the throw block
(ActGThrow / BtlCollThrow / the thr* fields: one BtlFighterThrow, unkEC4 / unkEE8 = thr.failed /
thr.halfDamage, unkE9C = thr.slot, unk5C = raiseAtEnd, its comment fixed), the reaction block and the
timers it overlaps (they are one BtlFighterReact), the pose views (velY, speed / facing at +0x90 /
+0x94, vel / dir, move / moved: one BtlFighterPose), FxChr hitPos / hitKind (aimDir / blastNode),
+0xE00 / +0xE04 (dodges / dodgesB), unkD88 (evasionCount), unkD44 (combo.hits), BtlActJForm.unk24..30,
the member gauge word +0x20 (gauge.variant) and the damage queue words (BtlFighterQueue).

## Still open

Reported by the five agents that proposed the names of docs/names/renamed_2026_10_08.tsv. Not acted on yet,
except where a rename of that list covers it. What is left below could not be settled from the code, or would change it.

CPU opponent:
- Two AI functions were left as Unk (step 23, condition 11): both hinge on "stage 4 or 27" and pose bit 0x80, whose meaning is not known.

Effects, stage, objects:
- Setters left as Unk: HudNode_SetUnk8 (HudDNode +0x08 / +0x0C, which HudNode_Draw does not read) and EftBill_SetUnkD8
  (EftBill +0xD8: when > 0 at the end, flag 0x800 is set, and nothing tests 0x800). (EftRibbon_SetUnkD8 is EftRibbon_SetNode;
  EftZap_SetUnk40 no longer exists.)

Fighter:
- "Body changed" (the member flag at +0x70, technique 0x280, character 0x56) is a reading, not a confirmed name.

## From the field-name groups

Group 06 (btl_script_cmd.h .. eft_disc.h):
- Ten MEDIUM names in btl_tech.h follow the existing getter's name only (poweredDashLimit, landingKind, throwObjectSlot, ...).

Group 08 (eft_obj_tech.h .. eft_quad_2.h):
- eft_part10.h:151, 156: layer (0x1FB) / drawMode (0x20C) are blend / mode in eft_link_2.h: which is right was not determined.
- eft_obj_tech.h:247: EftObjTech.nodes[2] is the 0x10-byte header of the EftEmitNodes block plus slot 0's id and padding, not two positions;
  the Vec4 arrays unk390[3] (line 249) and EftRushShot.unk360[3] (line 306) are id / padding / position mixes.

Group 07 (eft_draw_modules.h .. eft_link_2.h):
- eft_draw_modules.h:103: EftChainStrand.mtx (Mtx44 at 0x00) is four vectors in EftArcChain (pos, drift, unk20, color).

Group 10 (eft_stage_2.h .. eft_water.h):
- eft_trail.h:72, 77-78, 83 against eft_disc.h: the views of the glow work disagree (paramFlags / type / type2 / fadeFrom against unk8 /
  unk1C[2] / alpha); eft_trail.h:126 EftGlowMgr.live at +0x18 is unk18[2] in EftPGlowMgr. Not determined which is right.

Group 03 (btl_char_action.h .. btl_char_cam.h):
- BtlAct_IsAirMotion (btl_char_action.c) returns, with useSaved, react.back (fighter +0xFBC, hit from behind) and otherwise motion
  flag 0x8000: the comment says so, but what the function as a whole answers (and so its name) is not settled.

Group 05 (btl_char_mgr.h .. btl_script.h):
- BtlParam.unk4[8] (+0x04..+0x0B) against BobjParam height (+4) / radius (+8): BtlParam has its own `radius` at +0x0C, so which
  word is the radius is not settled (BobjParam took charaFlags / sizeClass / auraKind at +0..3 in names-pass-3).
- BobjPart.active (+0x0C) is s32 here and `u8 active` + padding in btl_obj.h / btl_obj_anim_part2.h: the code stores words through
  this view, so a u8 would change it. (BtlMoveBlastRec.atk and pose.groundFlags were settled earlier.)
- DemoCamPose.unk1C sits in the anonymous struct `f` inside the union.

Group 12 (overlay_common.h .. memcard_flow.h; the menu overlay and sys):
- gProgress +0x00 `language` is a reading (it picks the story text file set and is added to HUD file 6).
- McCardIconSys.unk4 / unk8 -> reserved1 / reserved2 rests on the agent's memory of Sony's sceMcIconSys, not on the repo.

Group 11 (eft_water_part2.h .. option.h; stage, HUD, object renderer, menu overlay):
- Relocated but never read: StgTimer.unk4, StgColZone.unk10, StgOctNode.unk00 / StgMCell.unk0.

Group 20 (names pass 3: engine, menu, effects / stages, battle; lists in docs/names/fields/group20_*.tsv):
- The compiler's output can depend on declarations a file does not use: declaring the survival record type
  (SaveSurvBest) in sys/save.h changed one address computation in src/menu/team_select.c (`gPad+396` became
  `addu 384` + `gPad+12`), though team_select.c never names it; one or two typedefs fewer and the code is the
  same again. SaveData.survival stays u8[5][0xC]. A header change that builds is not proof that the next one
  will: always build.
- SaveData.course (+0x780) is sized 5 (the courses ub.h / ub_rank.h index); menu sim_day.h / ub_score.h size the
  same array bestC[54] and UbScore_SaveBestC is "the courses of modes 24..30". To be reconciled. SaveData.simCleared
  is u8 in menu/sim_top.h, s32 elsewhere.
- Not applied: DetAiRule.unk4 / unk8 (cond halves of AiThRule.cond[8], but BtlAiSeq_PushRule reads act[] / arg[]
  through them out of range), BtlTechBParam.unkC7 (damage percent of technique 0x2D5 against the character; which
  technique that is was not found), BtlReplayData.unk0 (two BtlReplayTrack; the u64 view is kept for the copy's
  alignment).
- Names applied on one reading (MEDIUM): Job.link, ViewProgress.prevMode / lastLoadType, Option.exitSaving,
  EftAuraCfg.powerPush, EftAuraPrm.nearWeight, EftSetHead.moveFlags (mixed movement and firing bits),
  EftRibbon.node (nothing reads it), the record word `stationary` (+0x54 of the hit record, five views; held
  technique or ki bomb at rest), FxHitArg.texVariant / FxHitArg2.texVariant, BtlObjBound / BobjBone / BObjBound
  drawFlags (s32 over ObjDrawPart's u16 flags + u16), ObjDrawPkt.texRegHi, DemoCamAnim.extraData, the
  TextBox draw* fields (0x50..0x8B is a TextBoxDraw; replacing the flat fields by `TextBoxDraw draw` would be
  cleaner), Num_DrawEx parent / child (with child NULL, parent is the format of the digit clips themselves).
- Views that should be replaced by the real structure: TextBoxClipProp (FlashProp), DialogFlash (Flash), the ui
  Flash view (sys/flash.h Flash).
- Reported, not changed (would change what the compiler sees): a possible argument swap in an EftPrim_DrawTriangle
  call near src/battle/eft_core.c:1663 (EftGfx_DrawPolyScaledZ).
