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

Reported by the five agents that proposed the names of docs/names/renamed_2026_10_08.tsv. Not acted on yet,
except where a rename of that list covers it. For the pass over fields and local variables.

CPU opponent:
- Two AI functions were left as Unk (step 23, condition 11): both hinge on "stage 4 or 27" and pose bit 0x80, whose meaning is not known.

Effects, stage, objects:
- Setters left as Unk: HudNode_SetUnk8 (HudDNode +0x08 / +0x0C, which HudNode_Draw does not read) and EftBill_SetUnkD8
  (EftBill +0xD8: when > 0 at the end, flag 0x800 is set, and nothing tests 0x800). (EftRibbon_SetUnkD8 is EftRibbon_SetNode;
  EftZap_SetUnk40 no longer exists.)

Fighter:
- BtlAnim_SetRate (btl_anim.c) writes fighter +0x988, the model scale that BtlChar_PoseToObj copies into the object transform
  (the uniform scale of its matrix): a scale setter, misnamed (found by the fighter merge, 2026-10-25).
- btl_act_1.c, btl_act_2.c, btl_act_super.c, btl_act_change.c, btl_char_fx_1.c, btl_char_api_2.c still re-declare some fighter
  functions with cast macros that change an object or parameter type (79 left after the merge removed the 218 identical ones).
- BtlCharApi_GetTechniqueProgress (btl_char_api_2.c, about line 1151) is misnamed: (event frame - current frame) / step = updates left until an animation event.
- btl_char_api_1.c:335 "No caller" for the character pack getter: BtlScene_GetCharPackEntry (btl_scene.c:254) calls it; the prototypes disagree (s32 / s32 *).
- btl_obj_anim.c, above BtlObjFace_PickJawKeys: "eye direction tracks" are jaw rotation tracks (node 0x31).
- btl_act_super.c:2097: technique 0x280 "a random character": the character comes from the technique table.
- btl_scene.c:548 "rule word 0x18" is rule word 0x10 (setup + 0x18).
- Fields to follow the renamed accessors: unk44FC4 (default animation step, btl_obj.h; its comment "getter / setter only" is out of date), unkB30 (alphaAdd), rule.unk10,
  BattleSide.unk1FC / unk200 and the parameters of BattleSetup_SetSide in the menu files (team_select.c calls the first `handicap[]`).
- btl_char_fx_1.c, BtlFx_FireKiBlast: the local `react` holds ki blast record +0x31, the volley spread mode.
- BtlCharApi_GetOppSkillKind reads the technique table (slots 2..4) and BtlCharApi_GetOppMoveKind the skill table (slots 0..1): the names are
  crossed relative to the module prefixes; the extern comments at btl_char_api_2.c:912 / 914 have the same mix-up.
- btl_stats.h has unk14C where btl_obj_anim.h has mix.
- Accessors still named by offset that have named fields: BtlParam_GetSlotA0 / A4 (transSeq / transKind), BtlParam_GetCostAE (fusionCost),
  BtlOpp_GetParamByte2, BtlOpp_GetParamWord0, BtlCharApi_GetParamByte84 / 8A / 8D, BtlCharApi_HasParamBit80.
- "Body changed" (the member flag at +0x70, technique 0x280, character 0x56) is a reading, not a confirmed name.

System, menu support:
- src/sys/adx.c:17 cites an array at 0x334788 [voice][kind]: it is gBtlScript.voice (0x333BC0 + 0xBC8), indexed [side * 2 + window].
- include/sys/game_pad.h:75: the table at 0x2EF050 is the rodata copy of the local padBit[] in BtlInput_GetKeyMask, not a global.
- include/sys/gfx_screen.h:92: an unused extern for the constant-pool word 224 of GfxWater_DrawView: delete.
- src/sys/rigid.c:423: the "variable" at 0x2FE818 is a .lit4 literal (3 pi / 4).
- include/ui/reward_window.h: its TextBox struct has unkC, unk10, unk50, unk80, color2, rect where TextBoxFull (menu_support.h) has x, y,
  draw.align, draw.noFlush, shadow, clip: TextBox_SetColor2 is the shadow colour, TextBox_SetRect the font clip rectangle.
- src/ui/menu_util_1.c: the comment on TextBox_SetMaxSize mentions an `unused` parameter that does not exist.
- src/sys/memcard.c: the header comment still calls the file stem a placeholder.
- src/sys/vu1_packet.c: the header numbers the second variant of program 2 as "prog 3"; the listings and docs call it 2b.

## From the field-name groups

Group 02 (btl_act_super.h .. btl_cam.h):

Group 09 (eft_rays.h .. eft_stage_1.h):

Group 01 (battle.h .. btl_act_decide.h):
- Found: BattleRule.unk18 and BattleResult.unk44 are the Dragon Ball drop (1 + the first unset of the seven dragon-ball bits; stg_parts.c
  moves it to the result with a 1-in-8 chance when a stage object breaks; history_result.c sets the bit). BattleRes.unk8 is the HUD sprite file.

Group 06 (btl_script_cmd.h .. eft_disc.h):
- eft_aura.h, EftAuraCfg.flame[EFT_AURA_SPARKS] at 0x410: the table has 10 entries (EFT_AURA_PARTS); entries 10 and 11 overlap fadeNode[].
- eft_aura.h, EftAuraCfgFlame.unk0 (u8) and .node: in EftAuraDataPart the record is s32 node, nodeEnd, nodeRef: this `node` is the other's nodeEnd.
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

Group 04 (btl_char_coll.h .. btl_char_member.h):
- btl_char_ctl.h:117-118: BtlCtlNode.mtx at +0x50 is the parent's world matrix (the node's own is at +0x10); `pos` at +0x90 is the local translation.
- btl_char_coll.h:165 and btl_char_member.h:98: fighter +0x00 is `side` in BtlCollChr / BtlMemberChr, `player` (and +0x08 `side`) elsewhere.
- btl_char_coll.h:150, 153: BtlCollReact.dirYaw / faceYaw are yaw / turnYaw in HitReact and ActGReact, `id` is `reaction`; unk18 is s32 here, f32 scale there;
  its four two-word arrays (unk4[2], unk10[2], unk28[2], unk3C[2]) each cover two different fields of HitReact.
- BtlFlagPose.unk30 (btl_char_flag.h:119) is `vel` in BtlCapiPose, but this structure uses `vel` for +0x40 (FxPose too; BtlCapiPose: move;
  BtlActBPose / ActGPose: moved): the views need settling together.
- Competing names chosen: skill block 0xE00..0xE18 after BtlActIChr (skillTimer, skillSlot, skillKiRate, skillTimerC), HitChr.unkE18 noFlinch
  (BtlActIChr: skillTimerD; BtlCapiBChr 0xE0C: dodgeKind); 0xE44 techCharge (BtlSuperChr: charge) against the attack charge at 0xD78;
  pose +0x20 dispOfs (BtlCtlPose: move); object +0xCAD hitNo (BobjHit: hitIndex); the six defence timers 0x1068..0x107C named from
  BtlMove_UpdateDefenseTimers (dodgeWindow, blastDodgeWindow, counterWindow, deflectWindow, throwBreakWindow, rushBreakWindow), all uncertain.

Group 03 (btl_char_action.h .. btl_char_cam.h):
- Pose +0x90 is the heading pitch (BtlMove_SetHeading; six views; combat.md line 445), named `speed` in BtlActPose, BtlActCPose, BtlActEPose,
  BtlCtlPose; the real speed is +0x98. Also +0x80 `vel` is `dir`, +0x94 `facing` is `yaw`.
- btl_char_action.h:91-92: BtlActVitals gaugeB / gaugeBMax are ki / kiMax. :161: switchUnk20 is the member's model variant.
- btl_char_action.h:272: BtlAct_IsAirMotion with useSaved returns fighter +0xFBC, which is HitReact.back (hit from behind).
- btl_char_api_2.h:110: BtlCharApiMgr +0x0C `sounds` is the looping set (the one-shot set is +0x08). :90: attrCount at +0xCAE is eventCount.
- btl_char_api_2.h:35-45: camUnk420 / 460 / 494 / 4A0 are ChrCam.eye, bodyPos, hit, yaw. btl_char_api_1.h:76: flags0 is charaFlags;
  :58-63 thrUnk10 / 1C / 48 / 4C are still unknowns under a prefix.
- Fighter +0x00 / +0x08: player / side (BtlActChr, BtlCapiChr, BtlCapiBChr) against side / index (BtlMgrChr, BtlMemberChr, ChrCamChr,
  BtlCharApiChr): one must win before a shared fighter header.
- Choices: +0xE40 techDelay (BtlSuperChr: cooldown, and the accessor is ...GetTechniqueCooldown); attack block +0x09 leadIn (BtlActBAttack: level);
  +0x974 motion (BtlStatChr: anim.cur); +0x1550..0x1558 scriptMotion / Blend / Loop in BtlActChr (BtlCapiBChr: motion / motionBlend / motionLoop).

Group 05 (btl_char_mgr.h .. btl_script.h):
- btl_char_mgr.h:173: padFlagA (+0x15D0) is BtlCharVib.enabled; the following unk15D4[5] is the rest of that structure (power, time,
  smallTime, phase, toggle; the first an f32): the pair is better replaced by BtlCharVib.
- btl_char_mgr.h:187: the comment on unkA0 says "four 0x20-byte records at +0xB0": the records start at +0xA0 (BtlFlagSndReqList.req).
- btl_char_mgr.h placeholders that are not `unk`: prev964 (+0x968) prevActionFrame, prev974 (+0x97C) prevMotion, prev1262 (+0x126B) prevFxBits,
  new20 (+0x12E0) newVariant. :130 unkE00[5] covers five words other views name separately (and disagree on: dodges / skillStackA, ...).
- btl_char_api_2.h:282: switchPrompt (+0x1594) is the technique class being watched (techClass in btl_param.h, inputClass in btl_act_super.h).
- btl_obj_anim.h BobjParam is BtlParam (object +0x91C): unk00 is typed f32 but overlays charaFlags (u16), sizeClass, auraKind; height (+4)
  and radius (+8) fall inside BtlParam.unk4[], which could take those names.
- btl_obj.h:146: BtlObjMdl.anims (+0x98) holds the camera animations DemoCam_PlayObjAnim plays; the motion table is BObjMdl.anims at +0xA8.
- BobjFace (btl_obj_anim.h) against BObjFace (btl_obj_anim_part2.h): blinkTime / blinkFrames, jawKeys / jawTable, talkKeys / talkTable.
- btl_char_move.h:110: BtlMoveBlastRec.def (+0x68) is `atk` in EftHitRec / EftDetRec, with `src` at +0x64. :53: BtlMovePose.unkD0 is f32
  here, `s32 groundFlags` in BtlCollPose. btl_obj_anim.h:117: BobjPart.unk0C is s32 here, `u8 active` + padding elsewhere.
- Choices: fighter +0x1594 techClass; +0xE14 / +0xE18 skillTimerC / skillTimerD (btl_char_coll.h: noFlinch for +0xE18: group 04 chose that);
  object +0x1660 charaWork (four views: work); object +0xFA0 bodySphere (EftDetObj: bodyPos); object +0x14 res (BObj: slot).
- DemoCamPose.unk1C sits in the anonymous struct `f` inside the union.

Group 12 (overlay_common.h .. memcard_flow.h; the menu overlay and sys):
- battle_setup.h:133-134 (and sim_day.h:49-50, survival.h:44-45): the prototype parameters unk10, unk1FC, unk200 are rule->stageChange,
  side->changeAllowed, side->switchEnabled; the comments "BattleSetup_SetRule's last argument" / "side 1's unk1FC" in sim_day.h:184, 188,
  survival.h:153, 157, ub.h:346, 350, ub_rank.h:122, 126 are stale.
- ub_score.h:145: UbScore.line "health, unk24, unk1C, battle time": those are maxComboHits and maxComboDamage of BattleResult.
- memcard_flow.h:56-61 against memcard.h:47-61 describe the same request block differently (title.text[0x100] + s32 titleBreak against
  title[0x44], iconName[3][0x40], u16 titleBreak): McFlowReq.unk418 is the tail of iconName[2].
- tour_entry.h:281-285: TourInfo has prize[2][3] + unk28[15]; tournament.h:216-219 has the same record as prize[7][3].
- sim_day.h:101 and ub_score.h:215: `s32 unk288` where sim_top.h:114 has `u8 simCleared`; unk778[2] element 1 is 0x77C, the ladder rank.
- gProgress 0x640 has three names: cursor (ub.h, survival.h), ubKind (ub_rank.h:71), misRow (ub_score.h:122); 0x684 is discFlags / ubFlags.
- sys/save.h:77, 80: unk208 and unk77C are ubFlags and rank in the menu views; unk20C ("a capacity, guess") is missionPages.
- gProgress +0x00 `language` is a reading (it picks the story text file set and is added to HUD file 6).
- McCardIconSys.unk4 / unk8 -> reserved1 / reserved2 rests on the agent's memory of Sony's sceMcIconSys, not on the repo.

Group 11 (eft_water_part2.h .. option.h; stage, HUD, object renderer, menu overlay):
- dc_password_replay.h:95: ZaChrEntry.slots at +0x0E is baseLevel in ChrTblEntry and VChrEntry.
- evo_z.h:124 against evo_z_items.h:165: +0xB0 of one work is `bg` in UEvoZ and chipTex in EvoZ; +0x1A64 `fits` has two opposite comments.
- history_result.c:59 still says "BattleResult.unk44" (dragonBallFound).
- The 0x3C-byte blocks DcList.unkC44, DcMenu.unk88, DcPassView.unk17C, Option.unk1B0 are only passed as a fourth argument MsgWin_Init does not take.
- Relocated but never read: StgTimer.unk4, StgColZone.unk10, StgOctNode.unk00 / StgMCell.unk0.

Group 13 (movie.h .. shen_scene.c; sys, ui, and the structures local to C files):
- Locals named unk… (not fields): battle_load.c:652 unk1C (anim1Chara), btl_char_fx_1.c:173 unkA24 (area), btl_char_cam.c:958-959 and
  1028-1033, btl_obj.c:569, 571, eft_char_parts.c:272-273, eft_ribbon.c:674, 676, char_select.c:1560, mission_select.c:52-53,
  survival_select.c:26-27.
- src/battle/eft_burst.c defines EftView twice (lines 65 and 1135).
- menu_support.h:101 ChrTblEntry.baseLevel (+0x0E), VChrEntry.baseLevel (evo_z_items.h:78), against ZaChrEntry.slots: PassChk_IsValid checks
  slots + extraSlots <= 7 and DcPass_Decode stores extraSlots into rec.level: the saved "level" looks like extra item slots and "baseLevel"
  like the base slot count. To be reconciled.
- eft_link_2.h:165: EftWLinkArg.frame (+0x38): the only caller (eft_sweep.c:397) passes the texture index it gives EftPart10Arg.texIdxA.
- eft_obj_tech.h:167: EftKCamArg.pack (+0) is `anim` in EftCamArg (eft_core.h:210).
- hud_combo.c:83 skillScript against pause_menu.c:50 skillText for object +0xBC (text entry 0x2D + language: skillText fits).
- duel.h:93 `versus` against `players` (team_select.h:76, char_select.h:125) for progress +0x620.
- menu_support.h:200: MenuUtilProgress.team[7] of ProgressTeam at +0x69C are replay slots (ReplaySlot replay[], McFlowSlotInfo slot[8]): the
  count differs (7 against 8).
- ub.h:132 and ub_rank.h:100 still call save +0x208 unk208 (ubFlags elsewhere). eft_zap.h:122, 124: EftZapArg.unk38 / unk40 are texIdx / type.
- docs/systems/save_data.md:22 and menu_overlay.md:795 say save +0x1694 / +0x1698 are unknown: the camera distance preset and camera shake off.
- sys/save.h:63: SaveRec.unk0[0x14] hides u16 item[8] and a word at +0x10 (ZSaveRec in dc.h:81).
- Type differences behind a name: McCardInfo.unk20 (s32 / void *buf), BObjCommon.unk00 (s32 / void *boot), EftFlashArg.unk14 (s32 / f32 hold),
  HudPromptObj.unkBC (s32 / pointer), EftArg12.unk40 (a u8[1] array).

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
- Types the new names show to be wrong (not changed): StgFxRes.count0..2 are counts typed void * (their getters
  BtlStage_GetFxResA / B / C return void *); EftHStartArg.scale is s32, f32 at the sender; EftKGroup.parts /
  EftOGroup.parts / EftJGroup.parts are s32 for an EftSetDef *; EftWaterView.tex[].image is a u64.
- Function names left: EftRibbon_SetUnkD8 sets the node (EftRibbon_SetNode); BtlStage_GetFxResA / B / C / A2 / B2 /
  C2 return count0 / list0 ...; BtlSkill_GetShotUnk30 / BtlSuper_GetShotUnk6C return the shot scale;
  BtlSkill_GetUnk14 the clash power; BtlParam_GetUnkC7Scale.
- Views that should be replaced by the real structure: TextBoxClipProp (FlashProp), DialogFlash (Flash), the ui
  Flash view (sys/flash.h Flash).
- Reported, not changed (would change what the compiler sees): a possible argument swap in an EftPrim_DrawTriangle
  call near src/battle/eft_core.c:1663 (EftGfx_DrawPolyScaledZ).
