# Wrong or stale names and comments found while naming placeholders (2026-10-08)

Reported by the five agents that proposed the names of docs/names/renamed_2026_10_08.tsv. Not acted on yet,
except where a rename of that list covers it. For the pass over fields and local variables.

CPU opponent:
- btl_char_api_2.c: the comment on the getter of fighter +0xD74 says "(no caller)"; BtlAiSeq_RollPowerUpChain calls it.
- btl_char_api_2.c: the comments on the two +0x974 getters are stale: they return BtlAnim_GetId(chr) and BtlAnim_GetFlags(BtlAnim_GetId(chr)).
- btl_ai_seq.c, AI steps 18 / 19 / 21: the local `dist` holds BtlCharApi_GetHp (compared with 10000); the tables `near` / `far` are low /
  high health; the comment on handler 18 says "by level and distance": it is by level and own health. The same in btl_ai_think.c near line 1199.
- btl_ai_seq.c, steps 15 and 23: the local `busy = BtlAiStep_ReachClass(ai)` is inverted (1 = class reached).
- AI locals named `state` that hold the animation id; the AI data's stateClass / stateFlags tables are indexed by animation id.
- btl_char_api_2.h param view: unk14 / unk18 are flags2 / flags3 (btl_param.h); unk1288 is actBits (btl_char_coll.h); unkCAD is hitNo.
- Two AI functions were left as Unk (step 23, condition 11): both hinge on "stage 4 or 27" and pose bit 0x80, whose meaning is not known.
- docs/systems/ai.md: old file names, "still assembly" statements.

Effects, stage, objects:
- btl_char_api_2.h:59 `f32 unkEFC[4]; /* length not known */` is `f32 stepFrame[5]` in btl_act_super.h:305.
- The comment above the rush-sequence frame getter (btl_char_api_2.c, about line 503) is wrong: animation frame plus the lengths of the earlier steps.
- Fighter +0xD50: unkD50 in btl_param.h:191 and btl_char_mgr.h:130 is BtlMemberCombo.changed (btl_char_member.h); +0xD4C unkD4C is comboNewHit.
- Fighter +0xE60: contradictory comments across headers; the code adds 1 (btl_act_change.c:160) and 5 (btl_act_super.c:557), clamped to 0..10.
- BtlParam_GetGaugeB returns kiRecoverGoal: a weak name.
- The member gauge word +0x20 is described as "member +0x60" in btl_char_api_2.h:22; BtlMemberQueue.unk20 (btl_char_member.h:90) is another thing.
- eft_quad_2.h:16 and :495 call the class at 0x2C40F0 the "EftLine" / "line task" class; its callbacks are EftBill_*.
- The class at 0x2C3EC0 has EftShotFxMgr_Init as its init among EftShotFx_* callbacks: one prefix is probably wrong.
- eft_trail.c:728 declares the class at 0x2C3E00 as u8[0x18]; the other class tables are void *[6].
- eft_detect.c:157 and stg_collision.c:96 describe the vector at 0x2EC2E0 differently, with two struct types.
- Setters of fields nothing reads were left as Unk: EftRibbon_SetUnkD8, EftZap_SetUnk40, HudNode_SetUnk8, EftBill_SetUnkD8.

Fighter:
- BtlCharApi_GetTechniqueProgress (btl_char_api_2.c, about line 1151) is misnamed: (event frame - current frame) / step = updates left until an animation event.
- btl_char_api_1.c:335 "No caller" for the character pack getter: BtlScene_GetCharPackEntry (btl_scene.c:254) calls it; the prototypes disagree (s32 / s32 *).
- btl_obj_anim.c, above BtlObjFace_PickJawKeys: "eye direction tracks" are jaw rotation tracks (node 0x31).
- btl_act_super.c:2097: technique 0x280 "a random character": the character comes from the technique table.
- btl_scene.c:548 "rule word 0x18" is rule word 0x10 (setup + 0x18).
- The two zero vectors at 0x2EC2A0 / 0x2EC2C0 each cover 0x20 bytes: a second unreferenced vector follows each ((1,1,1,1) and (1,0,0,1)).
- Fields to follow the renamed accessors: unk1310 (body warp), camUnk460 / cam.unk40 (camera body position), unk1294, thr.unk14 / 18 / 2C,
  unk44FC4 (default animation step, btl_obj.h; its comment "getter / setter only" is out of date), unkB30 (alphaAdd), rule.unk10,
  BattleSide.unk1FC / unk200 and the parameters of BattleSetup_SetSide in the menu files (team_select.c calls the first `handicap[]`).
- btl_char_fx_1.c, BtlFx_FireKiBlast: the local `react` holds ki blast record +0x31, the volley spread mode.
- BtlCharApi_GetOppSkillKind reads the technique table (slots 2..4) and BtlCharApi_GetOppMoveKind the skill table (slots 0..1): the names are
  crossed relative to the module prefixes; the extern comments at btl_char_api_2.c:912 / 914 have the same mix-up.
- Fighter +0x964 is unk964 / unk968 / prev964 in btl_char_flag.h and btl_char_mgr.h, actionFrame in btl_act_1.h and btl_act_2.h.
- Several headers have unkE44 where btl_act_super.h and btl_tech.h have charge / techCharge; btl_stats.h has unk14C where btl_obj_anim.h has mix.
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
- Arrays that span several named fields and want splitting, not one name: BtlAiSeq.unk5C[3], unk6C[2], unk8C[3]; AiThSeq.unk5C[3];
  BtlAiSeqA.unk6C[2], unk8C[3]; AiActSeq.unk0C[2]. BtlAiWork.unk10[2] is costMin / costMax in AiThWork.
- BtlAiPlan has two unk4: the plan's own (+0x04, `next`) and the one inside the anonymous rolls[] struct (`acc`): the second by hand.
- Types differ between views: BtlAiOutput +0x48 / +0x4C (accX / accY) s32 here, f32 in AiActPad; status +0x24 (act10Dist) s32 / f32.
- ActGThrow is the same block as BtlCollThrow (btl_char_coll.h): names for +0x10..+0x64 to be reconciled; also btl_char_api_1.h thrUnk10 / 1C /
  48 / 4C, btl_stats.h unkEE4, BtlMemberQueue unk0 / unk14 / unk20 (totalStart, drainHealthStart, drainKiStart).
- btl_act_super.h:84 / :86: unkEC4 and unkEE8 are words +0x34 / +0x58 of the throw block at +0xE90, filled in BtlColl_StartThrow.
- btl_act_super.h:25-26: BtlSuperPose calls +0x9C velY; every other pose view has +0x98 speed, +0x9C fallSpeed (positive = down).
- btl_act_super.c:2108: comment says unk5C puts the attacker "on the ground when the action ends"; the code at :2437 raises the fighter.
- btl_ai_sense.h AiActStatus.downTimer (+0x08): set to 4 while an attack hit is pending, then counted down: nothing about being down.
- btl_act_decide.h:83 unkE9C sits at throw block +0x0C, which BtlCollThrow names `slot`: not checked whether the descriptions agree.
- btl_char_ctl.h:78: the sixth parameter of BtlChange_RequestChara is named unk18 but receives form +0x1C.
- btl_char_hit.h HitReact: the comments for unk38 and unk48 describe fighter +0xFE8 / +0xFF8, which other headers describe as countdowns:
  the react block and those timers overlap, one of the two layouts needs a look.

Group 09 (eft_rays.h .. eft_stage_1.h):
- Technique definition +0x04 / +0x05: eft_core.h and eft_shot.h have cls@4, kind@5; eft_emit.h and eft_obj_tech.h have kind@4, unk5 / sub@5.
  EftShot_BuildParam shows +0x04 is 0 skill / 1 technique / 2 ultimate. One scheme should be picked for all views.
- Texture set word +0x204: `stepped` (eft_disc.h), `kept` (eft_sprite_anim.h:116), `uploaded` / `loaded` / `ready` in the 16-entry sets.
- Texture entry `u64 unk8`: other headers declare `void *image` plus `s32 unkC`: retype, not only rename.
- EftUPtclDef.unk280[12] would be better split into the twelve fields eft_particle_unused.h names (angX, angXRange, spinX, ...).
- EftMulti.unk390[4]: element [0] is the `hand` vector of EftBlast / EftPropShot: needs a split.
- EftJFlashArg.unk14 -> hold is s32 in this view, `f32 hold` in eft_trail.h.
- eft_shot_tech.h:40 `s16 recType; /* copied to the hit record's type */`: eft_core.c:122 does rec->level = rec->src->def->level: it is `level`.
- eft_core.h:97: the comment on `cls` says 0 is the "rush" class; 0 is the skill class.
- eft_shot_tech.h:80, 82: EftJPart has type@8 and kind@0xA; the same record in eft_sweep.h / eft_rays.h has kind@8, node@0xA: EftMulti_IsPartKind5
  most likely tests the node slot.
- eft_shot_tech.h:73: EftJSrc.objId20 is the field every other view names `chr`.
- eft_disc.h:104: the `hand` comment reads as inverted (eft_body_fx.c:344: non-zero means nodes 0x14 / 0x15).
- eft_zap.h:124 against eft_ribbon.h:291: the zap argument's +0x40 byte is unk40 in one and `type` in the other.

Group 01 (battle.h .. btl_act_decide.h):
- btl_act_2.h:22-23 (BtlActCPose) and :263-264 (BtlActEPose): +0x90 `speed` and +0x94 `facing` are the heading pitch and yaw (BtlMovePose,
  BtlActBPose, BtlActDPose, BtlActHPose: pitch@0x90, yaw@0x94, speed@0x98); `vel` at +0x80 is the unit travel direction (`dir` elsewhere).
  Their unk98 is the pose's speed and could not take the name while +0x90 has it.
- btl_act_2.h:18: BtlActCPose.move "y = fall speed": +0x40 is the whole movement of the previous frame (`moved` in BtlActBPose).
- btl_act_change.h:27: the comment on BtlActHPose.pitch cites "btl_char_ctl.h: speed", the same wrong name for +0x90.
- btl_char_fx_1.h FxChr: +0xDD0 hitPos / +0xDF0 hitKind conflict with the action code (the ki blast aim direction and the firing hand's node).
- btl_act_decide.h:76-77 against btl_act_change.h:468-469: +0xE00 / +0xE04 are dodges / dodgesB in four views, skillStackA / B in BtlActIChr.
- btl_char_action.h:185: unkD88 "frames with flag 8 in actions 0x89..0x8C": read through BtlOpp_GetEvasionCount, used modulo 3: a count.
- btl_act_2.h:70: BtlActCChr.unkD44 is the combo hit count (comboHits, BtlMemberCombo.hits).
- btl_act_decide.h:54-57: BtlActJForm.unk24 / 28 / 30 are objId / objCostume / objVariant of BtlActHForm.
- Found: BattleRule.unk18 and BattleResult.unk44 are the Dragon Ball drop (1 + the first unset of the seven dragon-ball bits; stg_parts.c
  moves it to the result with a 1-in-8 chance when a stage object breaks; history_result.c sets the bit). BattleRes.unk8 is the HUD sprite file.
- The +0x1C word of the form request / BtlJob (switchUnk18, form1C, the unk18 argument of BtlChange_RequestChara) is the character for the
  second resource file (0x599 + n * 10): `anim1Chara` proposed; the other spellings should follow.

Group 06 (btl_script_cmd.h .. eft_disc.h):
- eft_aura.h, EftAuraCfgSpark against EftAuraDataSpark: two views of one table shifted by 4 bytes; EftAuraCfgSpark.kind is the emitter node.
- eft_aura.h, EftAuraCfg.flame[EFT_AURA_SPARKS] at 0x410: the table has 10 entries (EFT_AURA_PARTS); entries 10 and 11 overlap fadeNode[].
- eft_aura.h, EftAuraCfgFlame.unk0 (u8) and .node: in EftAuraDataPart the record is s32 node, nodeEnd, nodeRef: this `node` is the other's nodeEnd.
- eft_detect.h, EftDetStageCtx.unk50 comment ("the place of the hit"): it is the sweep's movement vector (CpSweep.delta).
- btl_script_cmd.h, BtlScriptCmdWindow.color28 is FontStyle's shadowColor; its unk30[0x18] covers FontStyle's clip rectangle, userFlags and
  three callbacks; x / y at 0x48 lie past the 0x48-byte FontStyle.
- eft_disc.h, EftDiscAtk: 0x1B is `level` while FxHitArg2 / EftKiPropArg call 0x18 `level` and EftHitAtk / BtlCollAtk call 0x18 `kind`: two
  conventions for one launch block; its `node` at 0x14 is `code` elsewhere; objId at 0x12 against objId at 0x10 + objId2 at 0x12 in FxHitArg2.
- eft_core.h, EftHitAtk.level at 0x30: FxHitArg2 calls that word `size` (0..3).
- btl_tech.h, BtlKiBlastData.unk28: getter BtlKiBlast_GetRadius, but the effect views call the slot `scale`.
- btl_tech.h, BtlTechBParam.unk8F[0xC7 - 0x8F] runs over fields BtlParam names from 0x98 on (transTarget and others).
- The word after `count` in the texture sets: `stepped` (eft_disc.h, eft_rays.h), `ready` (eft_orb_tail.h, eft_part10.h), `built` (eft_chain.h).
- Ten MEDIUM names in btl_tech.h follow the existing getter's name only (poweredDashLimit, landingKind, throwObjectSlot, ...).

Group 08 (eft_obj_tech.h .. eft_quad_2.h):
- eft_part10.h:195: EftPart10Ptcl.speed (0xC0) is the particle's distance from the emitter (eft_link_2.h: dist); the speed / speedRange /
  speedBase tracks (lines 119-121, 263-265) are dist / distRange / distBase.
- eft_part10.h:212-215, 288-294, 166-169: the stretch* members are a colour multiplier pulse (eft_link_2.h: mulD / mul0 / mulT / mulTime);
  the flags EFT_PART10_P_SCALEX/Y/Z are per-channel pulse flags. The same for the part-9 quad at :410, 418-420, 486-488
  (colorOscAmp / colorMul / colorOscTime / colorOscPeriod in eft_quad_2.h).
- eft_part10.h:423: EftPart9Ptcl.size (0x13C) is the multiplier sizeMul; the base size is 0x138; dir (0x20) / vel (0x30) are vel / accel.
- eft_part10.h:242, 349: `rate` ("seconds between bursts") is a life in seconds (eft_link_2.h: life).
- eft_part10.h:151, 156: layer (0x1FB) / drawMode (0x20C) are blend / mode in eft_link_2.h: which is right was not determined.
- eft_part10.h:86: EFT_PART10_P_UNK80 is "the colour ramps", selected by def->unk20D.
- eft_obj_tech.h:247: EftObjTech.nodes[2] is the 0x10-byte header of the EftEmitNodes block plus slot 0's id and padding, not two positions;
  the Vec4 arrays unk390[3] (line 249) and EftRushShot.unk360[3] (line 306) are id / padding / position mixes.
- eft_obj_tech.h:78 against src/battle/eft_obj_tech.c:1212: the comment calls EftKPart.unk18 a life; eft_sweep.h names it `rate`.
- eft_tech_modules.h:166: EftRushShotWork.unk390 "position of the start node" is slot 1 (FIRE); slot 0 (START) is at 0x350.
- eft_particle_unused.h:373: "unk84 / unk90: jitter along the chain" has no backing in code. :443, 435-436: EftLinkNode.rot is rotZ,
  offset / pos are dir / origin in eft_link_2.h.
- Unused byte-array gaps with exact names in sibling views: EftPart10.unk29C = uv[16][4], EftPart9.unk298 = animSplit, EftPtcl.unk50 = accel,
  EftLinkNode.unk0 = corner[4].

Group 07 (eft_draw_modules.h .. eft_link_2.h):
- src/battle/eft_emit.c:1458 has a local twin of EftEmitLightArg, EftEmitRayArg, with the same unk20..unk50 fields: same renames wanted.
- Type problems, not names: EftSetHead.unkC[2] / unk30[2] are width1 / width2 and b1 / b2 in eft_sweep.h; EftHTask.unk28[4] is cls / prev /
  next / nextFree; EftVolleyShot.unk20 (Vec4) is {s32 script; f32 speed; f32 maxTurn} in eft_detect.h.
- EftSkillSrc.unk8 and EftSuperSrc.unkC are `flags` in btl_tech.h, but `flags` is taken by offset 0 (btl_tech.h: flagsA).
- eft_draw_modules.h:50: EftTTaskList.count ("live children") is BtlTaskList.head, a pointer; the test at eft_chain.c:2078 is "list not empty".
- eft_draw_modules.h:38-39: EftTTask.unk8 / unk18 comments ("texture animation state A/B") are wrong (BtlTask: pos at 0x10, list at 0x20).
- eft_draw_modules.h:103: EftChainStrand.mtx (Mtx44 at 0x00) is four vectors in EftArcChain (pos, drift, unk20, color).
- eft_emit.h:69: the comment on EftShotParam.unk5 says "super +0x141[i]": the code reads d->unk13B[n] (hitDirKind in btl_tech.h).
- eft_shot.h:57-58 against eft_emit.h / EftKDef: the two "kind"s are different fields: 0x04 is skill / technique / ultimate, 0x05 the sub-kind.
- eft_ground_dust.h:262: EftGndDustStage.unk54 sits at offset 0x10 (named after the field it is copied to).
- eft_link_2.h:352: EftPart10.stopDelay at 0x25C is spinB in eft_part10.h. :356: timer at 0x3A8 is `hold` there.
- eft_link_2.h:246: EftWLinkMgr.ptcl at 0x10 disagrees with EftLinkMgr.node at 0x20: both views of gEftLink cannot be right.
- eft_link_2.h:117-119: EftWLinkNode 0x40 / 0x50 / 0x60 are ofs / dir / origin; EftLinkNode has unk40 / offset / pos.
- eft_emit.h:409-410, 423-424: EftEmitArgA.texA / texB and EftEmitArg17.texA are definition blocks in the receiving modules; `res` is the texture set.

Group 10 (eft_stage_2.h .. eft_water.h):
- eft_shot_tech.h:42: EftJDef.recType is `level` (EftHit_Add copies definition +0x02 to rec->level; rec->type is the constant EFT_HIT_TECH).
- eft_shot_tech.h:347: EftJShotArg.evtIdx is a node slot index (EftBlastObj_Init stores it in sel[0].node).
- eft_sweep.h:170-171: EftEmitSet.tex33 / tex17 are the 16-texture and 8-texture sets (entries of 0x108 and 0x88 bytes): 33 and 17 look
  like size / 8; eft_emit.h has them as array[1] / array[2].
- eft_sweep.h:63 against eft_emit.h:292: definition +0x08 is `kind` in one and `phase` in the other; `phase` fits beside endPhase at +0x09.
- eft_stage_2.h:198-203 and eft_surface_out.h:63-73: ofs0 / ofs4 / ptr0 / ptr4 of EftSurfTex and EftBurstTex: the record has the layout of
  EftVramImage (eft_detect.h), +0x38 / +0x3C are image / clut; EftSurfTex.tex0 at +0x28 is the per-frame result, the record's own TEX0 is at +0x30.
- eft_surface_out.h:50: EftSurfD.param30 is `specular` (the last argument of EftGfx_LightClutSpecular).
- eft_stage_2.h:105: EftGeyserSteamArg.gravity is `accel` in EftSteamArg. eft_struggle.h:82: EftRList.first is BtlTaskList.head.
- eft_tech_modules.h:81: EftTechArg.side at +0x20 is EftHSlot.chr.
- eft_trail.h:72, 77-78, 83 against eft_disc.h: the views of the glow work disagree (paramFlags / type / type2 / fadeFrom against unk8 /
  unk1C[2] / alpha); eft_trail.h:126 EftGlowMgr.live at +0x18 is unk18[2] in EftPGlowMgr. Not determined which is right.
- src/battle/eft_stage_2.c:180-201: the local EftGeyserSmokeInit / EftGeyserSteamInit have the same unk30..unk50 fields as the header's
  EftGeyserSmokeArg / EftGeyserSteamArg and would take the same names.
- Definition +0x04 is `kind` in EftShotParam and `cls` in EftHitDef (which uses `kind` for +0x05): see group 09's note.

Group 04 (btl_char_coll.h .. btl_char_member.h):
- btl_char_ctl.h:117-118: BtlCtlNode.mtx at +0x50 is the parent's world matrix (the node's own is at +0x10); `pos` at +0x90 is the local translation.
- btl_char_coll.h:165 and btl_char_member.h:98: fighter +0x00 is `side` in BtlCollChr / BtlMemberChr, `player` (and +0x08 `side`) elsewhere.
- btl_char_coll.h:150, 153: BtlCollReact.dirYaw / faceYaw are yaw / turnYaw in HitReact and ActGReact, `id` is `reaction`; unk18 is s32 here, f32 scale there;
  its four two-word arrays (unk4[2], unk10[2], unk28[2], unk3C[2]) each cover two different fields of HitReact.
- btl_char_fx_1.h:150-162: FxHitArg2 against the callee's view EftRArg (eft_struggle.h:285): objId@0x10 is srcId, objId2 is objId, code is node,
  level@0x18 is kind, size@0x30 is level. :23: unk98 "reference speed" is the plain speed along dir.
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
