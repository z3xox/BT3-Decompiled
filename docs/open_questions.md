> **2026-10-08:** every function listed below as INCLUDE_ASM or unmatched now matches in C (docs/status.md, top).
> The open questions about them that remain are the fake matches listed there. File names below are the old ones
> (docs/file_rename_map.txt).

# Open questions

## Build and layout

- Most C files reference their global variables as `extern` from assembly. The ones that define their own
  `.sdata` so far: dialog.c, the HUD files (hud_a*.c), the menu windows (view_a*.c), and every file listed with a
  `.sdata` / `.data` subsegment in the yaml (the tenth step added the movie player, the memory card files, the rest of
  the HUD, the password codec, view_b_c / view_b_d and the wish screen).
- Original source-file boundaries are mostly unknown. Evidence so far: `.rodata` alignment (the
  object containing the battle sequence starts at 0x215540), and delay-slot behaviour that only
  matches when certain accessors are not defined earlier in the same file (`battle_work.c` vs
  `battle_load.c`).
- The CPU player's code is two objects. The second one's file-scope tables (0x2EDA70..0x2EDF08)
  sit between the two groups of function-local data, which puts the boundary after
  `BtlAiStep_Unk23` (ends 0x1B6B08) and before `BtlAiCond_React` (0x1B7188). Nothing narrows it
  further: the functions in between emit no read-only data and link identically on either side.
  The files are split at 0x1B6D50: `BtlAi_ScaleByLevel` (0x1B6D00) is on the first object's side,
  because two of its callers in the second object (`AiThink_GetSubRate4`,
  `AiThCond_ActRateByFlags`) only match with it outside their file. The first object is
  taken to start at 0x1B4140 (its first read-only item is the jump table at 0x2ED8C0); the data
  would equally allow an earlier start. The second object runs to 0x1BB128 at least
  (`AiThink_FindWeightColumn` needs the tables defined in its file, and the read-only data is
  continuous to 0x2EE248); whether `btl_ai_mgr.c` (from 0x1BB128) is a third object is not known.
- Other object boundaries placed by the same kind of evidence (a function that only matches with
  a callee defined, or not defined, earlier in its file): `btl_char_hit.c` runs to 0x1CA6D0;
  `btl_char_coll_b.c` starts somewhere in 0x1CA6D0..0x1CAEF0; `BtlColl_NextPoolMember` (0x1CDCA8)
  is in the same object as `BtlMember_Damage`, and the file split at 0x1CDCA8 is only the latest
  place the boundary can be (coll_b and member may be one object); `btl_input.c` runs to 0x1D60A0.
- More boundaries from the same evidence (action-handler batch): the effect request-bit helpers at
  0x1CF578 are in one object with the effect requests up to 0x1D3B40 at least (`BtlFx_UpdateGroundFx`
  needs `BtlChar_IsFxBitNew` above it), and the read-only data says that object does not start later than
  the member code's tables end, so `btl_char_member.c` and `btl_char_fx.c` may be one object;
  0x1E3158..0x1EA5F8 is one object (`BtlAct_AttackDashHandler` needs `BtlAct_RequestAttackEnd`), and
  continues `btl_char_action.c` by its data; 0x1F5460..0x1FC2B0 is one object
  (`BtlAct_SuperRushDashHandler` only matches in a file with the functions before it; which one it needs
  is not known); 0x1FC598..0x203168 is one object (`BtlAct_SwitchArriveLand`, `BtlAct_KoSwitchFlyIn` need
  `BtlActChange_SetFlags` / `BtlActChange_Finish`), and `BtlAct_Request` (0x1E0290) is not in it.
- Effect and stage batch: 0x13C300..0x13F430 is one object (`EftBurst_Update` needs the particle initialisers
  above it; the two caller-less tests at its end are its non-static inlines); 0x140338..0x147050 is one object
  (`EftWaterRing_Update` needs `EftWater_GetSurfaceY`), and nothing says where it starts (the steam emitters at
  0x13F430 are linked in the same file); 0x15AB38..0x15C728 is in one object with what precedes it
  (`EftRushShot_UpdateAttached` needs `EftRushShot_UpdateModels`). Against a merge: `EftBlast_PostUpdate`
  (eft_j.c) stops matching when eft_i.c is in front of it in one file, so there is a boundary in
  0x1532A0..0x155588 or before. Not tested: eft_c + eft_d, eft_g + eft_h + eft_i, eft_l_d + eft_m, the stage files.
- Second effects wave: 0x1793A8..0x17D290 is one object (`EftChain_SetRes` needs `EftChain_SetTex`), linked as
  0x178AB0..0x17EE68; 0x1809C0..0x182CE8 is one object (its read-only data runs tables, jump table, constants);
  0x182CE8..0x1871A8 is one object (`EftPtcl_SetTexture` needs `EftPtcl_PickTexture`); 0x195038..0x196E40 at
  least is one object (`EftBill_SetTexture` needs `EftLine_SetTex`); 0x1A0F40..0x1A3B00 at least is one object
  (four ribbon functions need helpers above them). The read-only data says more than the files do: if every
  object's `.rodata` was padded to 16 bytes (the rule the rest of the layout obeys), an object can only start
  on a 16-byte boundary; eft_w.c's data starts at 0x2ED2B8 and eft_y.c's at 0x2ED2D8, so eft_v_b.c (data from
  0x2ED1D0), eft_v_c.c, eft_w.c, eft_x*.c and eft_y.c (code 0x1871A8..0x195038) would be ONE object, and the
  next one starts with the line helpers at 0x195038 (data from 0x2ED310), which is where eft_y.c was cut.
  Nothing was merged on that evidence alone (the five files link identically as they are).
- Last batch (eft_ae, col_b / col_c, bobj, the AI sequence): 0x239BB0..0x23C310 is one object (`Font_Flush` needs
  `Font_BeginPacket` / `Font_EndPacket` above it); 0x24E9C8..0x24F9B0 at least is one object (`BtlObj_UpdateFace`
  and `BtlObj_IsJawActive` need `BtlObjFace_StepBlink` / `BtlObjMdl_HasJaw`), linked as 0x24BBE8..0x250B28, and
  it ends before 0x251940 (`BObjChainA_StepAll` / `BObjChainB_StepAll` match only when `BtlObj_GetNode`, 0x2505A8,
  is NOT defined in their file; the cut at 0x250B28 is a choice). 0x1ADBA8..0x1AE5F8 is one source file
  (`EftTexSet_Load*` on both sides of the eft_ae.c / eft_det_a.c cut need the empty `EftTexSet_CheckCount` known
  to clobber nothing); it is linked as two files with a `const` prototype in each. The AI sequence object starts
  at 0x1B3F78 (eft_det_b_c.c): its read-only data begins with `D_002ED8A0`. Compiling every remaining attempt of
  bobj_a.c, bobj_b_b.c and btl_ai_seq.c as a stub definition changes no other function.
- `BtlAct_SuperRushFollowHandler` (btl_act_f.c): whether its jump-table dispatch comes out in the
  original form depends on unrelated declarations earlier in the translation unit; it matches with a
  redundant prototype in front of it. What state of the compiler decides it is not understood.
- The 42 functions at 0x2BD230..0x2BF6B0, after the libraries, are game code (memory-card menu
  UI) that never uses `$gp`. Why they sit there, and whether they were built with different
  flags, is unknown.
- Two functions in the main body reach short `.sdata` strings with absolute addressing
  (0x1094A8..0x111358), suggesting objects built with different small-data flags.
- Assembler prelude gaps (see decomp_guide.md): a single-instruction load, store or `la` between
  a compiler-filled delay slot and an unfilled branch; `li.s` under `-G0`. (The `symbol(reg)`
  load in front of a return turned out not to be a gap: it depends on where the table is defined.)

## Functions that resist matching

Still pulled from assembly inside linked files:

| Function | File | What differs |
|---|---|---|
| `BtlInput_Update` | `btl_input.c` | 6 of 149 instructions: the emission order of twelve stores |
| `DemoCam_Update` | `btl_demo_cam.c` | 11 of 384: two saved registers swapped in one branch |
| `BtlText_DrawScrollBar` | `btl_seq.c` | one register swap |
| `Ot_Reset` | `gfx_ot.c` | the original copies an address through two extra registers |
| `Vu1Node_Animate` | `vu1_packet.c` | one instruction short; different loop induction variables |
| `BtlScene_TestCharPackBit`, `BtlScene_IsEffectHidden` | `btl_scene.c` | a saved-register choice; a switch's shared block |
| `IopHeap_PrintFree` | `iop_heap.c` | the original saves an unused register and does not tail-call |
| `Movie_ReadBuf` | `movie.c` | 5 of 44: order and register choice |
| `Sprite_DrawPicture` | `sprite.c` | 12 of 150: scheduling in the row loop |
| `Rigid_Init` | `rigid.c` | 2 of 72: two instructions swapped before a memset |
| `PadWatch_GetMissing` | `pad_watch.c` | 2 of 69: delay-slot fill |
| `ChrCam_CalcCut` | `btl_char_cam_cut.c` | 248 of 565: register allocation in the four "resolve a node" blocks; it owns two `.lit4` constants, which is why the fighter camera is three files |
| `BtlAiStep_FireSkill` | `btl_ai_seq.c` | 63 of 154: branch layout of the first half, registers of the class comparisons |
| `AiThink_TestSkill` | `btl_ai_cond.c` | about 260 of 308: eight strength-reduced pointers in the slot loop, four of them on the stack |
| `AiThink_GetBlastStep` | `btl_ai_cond.c` | 11 of 32: registers only |
| `BtlAiSense_IsBusy` | `btl_ai_act.c` | 8 of 48: a delay-slot fill |
| `BtlFx_SpawnSpeedLines` | `btl_char_fx.c` | 35 of 112: the store order and constant registers of the block that fills the 0x60-byte parameter structure |
| `BtlFx_FireKiBlast` (formerly `BtlFx_SpawnDamageSparks`) | `btl_char_fx.c` | register allocation only (two saved registers swapped, one value spilled); owns the jump table at 0x2EF020 |
| `BtlAct_GuardHandler` | `btl_act_c.c` | 2 of 298: the order of two argument loads in front of one call |
| `EftHit_InitMultiHit` | `eft_a.c` | 3 of 20 (which of v0 / v1 holds the definition) |
| `EftGfx_LightClutSpecular`, `EftGfx_DrawPolyAvgZ`, `EftGfx_DrawPolyFixedZ`, `EftGfx_DrawPolyAvgZFront`, `EftGfx_DrawPolyScaledZ`, `EftGfx_DrawSprite` | `eft_a.c` | 22 of 198, registers only; the polygon functions walk a pointer to `scr[i - 1][2]` in the original and to `scr[i][2]` here (7 of 115, 20 of 122), plus register allocation and the order of the clamps |
| `EftPrim_DrawBillboard`, `EftPrim_DrawQuadDepthScaled`, `EftPrim_DrawTriangle` | `eft_b.c` | about 50 of 370 (registers and scheduling of the corners); 8 of 200 (f29 / f30 swapped); 12 of 326 (scheduling of two constant loads) |
| `EftBubble_EmitBody`, `EftBubble_Add`, `EftGeyser_DrawColumn` | `eft_b.c` | the original does not strength-reduce the bone table index (owns the table at 0x2EC620); 5 of 151 (one store before instead of after the count increment); register allocation throughout (owns the table at 0x2EC6E0) |
| `EftGeyser_StartSmoke`, `EftGeyser_StartSteam` | `eft_c.c` | 17 of 162 and 19 of 126: the order of the stores that fill the emitter description |
| `EftSurf_DrawPolyOtClipped`, `EftSurf_DrawTriOt`, `EftSurf_DrawTriDirect` | `eft_d.c` | 458 instructions against 447; two too long; two short (the original keeps a value on the stack) |
| `EftBurst_DrawQuadRot`, `EftBurst_DrawModel` | `eft_d_b.c` | 22 of 274, all in the prologue (how the two 0x40-byte tables are copied); 561 instructions against 600 |
| `EftWater_AddSplashFor` | `eft_e.c` | 4 of 110: the square root's argument lives in f12 in the original and f1 here |
| `EftWater_IsOnScreen`, `EftWater_DrawBillboard`, `EftWater_DrawSprayQuad`, `EftWater_DrawClippedFan` | `eft_e.c` (second part) | inverse branch layout at the end of each case; 23 of 377 (registers of three constants); 126 of 456 (the roll loop counts up in the original); registers almost everywhere |
| `EftUtil_DrawTri`, `EftStorm_DrawBolts`, `EftStorm_SpawnBolt`, `EftStorm_DrawRainLines` | `eft_g.c` | two instructions swapped; 506 instructions against 636 (the original writes the second sprite twice); three instructions (a branch around `kind = 1`); slt / movn against slt / movz |
| `EftSmoke_Update`, `EftSmoke_Draw`, `EftBound_BuildWall`, `EftBound_Init` | `eft_g.c` | where the flag word is reloaded; two more float copies and a larger frame in the original; numbering of the saved registers; one store's position |
| `EftShot_BuildParam`, `EftVolley_Init` | `eft_h.c` | 499 instructions against 502 (address arithmetic shared differently); 6 of 72 (a repeated load of `slot->param`) |
| `EftEmit_SpawnType0`, `EftEmit_SpawnType16`, `EftEmit_SpawnType14`, `EftEmit_SpawnType5` | `eft_h.c` | saved-register numbering and a shared tail; 4 of 195 and 4 of 213 (s1 / s2 swapped); 16 of 191 (registers of the resource address) |
| `EftEmit_SpawnType9`, `EftEmit_SpawnType10`, `EftEmit_SpawnType15`, `EftEmit_SpawnType12`, `EftEmit_Spawn` | `eft_i.c` | 17 of 195, 25 of 207, 52 of 229, 56 of 205: where memset's arguments are set up in the block that fills the argument, and swapped saved registers; `EftEmit_Spawn` 66 of 753 (owns the two jump tables at 0x2EC990 / 0x2EC9B0) |
| `EftBlast_Init` | `eft_j.c` | 9 of 104: registers of three loads that gcse makes one pseudo; matches with `-fno-gcse` |
| `ScrXfade_StoreHalf`, `ScrXfade_Draw`, `ScrWarp_Draw`, `StgFog_Draw` | `stg_b.c` | 4 of 121 (scheduling before the first call); 25 of 145 (0x700 hoisted into a register in the original); 148 instructions against 150; 156 of 229 (schedule of six constants) |
| `StgHaze_Draw`, `StgBlur_Draw` | `stg_c.c` | GS packet loops: a second copy of `rows - 1` and reloads from the stack in the original |
| `EftAura_DrawFlames`, `EftAura_SetType`, `EftBolt_Shape`, `EftBolt_Draw`, `EftBolt_Spawn` | `eft_n.c` | 512 of 564; the original tests `flags & 0x80` twice from one register and re-reads `aura->type` after storing it (its attempt is compiled as a stub so that `EftAura_ChangeType` matches); 431 of 466 and 359 of 474 (same statements and frame, register allocation); 116 of 547 |
| `EftBolt_UpdateTex`, `EftRays_DrawRay`, `EftRays_Step`, `EftRays_LerpKey` | `eft_o.c` | 5 of 70 (when `t->tex0` is loaded around a call); 127 of 296 (frame 0x3F0 against 0x400, one more 16-byte local in the original); 8 of 185 (f21 / f22 swapped); 29 of 116 (a2 / a3 swapped) |
| `EftBlastObj_UpdateParts`, `EftBlastObj_Init` | `eft_o_b.c` | 70 of 115 (the original leaves the loop from both node tests with `skip = 0` in the delay slot); 6 of 172 (registers of the node slot copy) |
| `EftGlow_BuildTables`, `EftGlow_SpawnPart`, `EftGlow_PairPart`, `EftGlow_DrawParts` | `eft_p_b.c` | 23 of 205 (how the address of `dir[i].y` is formed); 73 of 546 (the original shares ONE `jal Vec4_Sub` between two arms); 57 of 120 (s2 / s3 swapped, three copies of one pointer); 9 of 487 (registers of the three off-screen tests) |
| `EftTrail_Draw` | `eft_q.c` | 320 of 507; dead code (nothing references its class) |
| `EftKiObj_DrawFrags`, `EftChain_BlendKeys`, `EftChain_DrawStrand`, `EftChain_StartStrand` | `eft_s.c` | 3 of 72 (`li s2,4` before the 2^31 constant in the original); 707 of 743 (frame 0x180 against 0x1A0, `out` in s6); 29 of 433 (a `beqzl` with the loop-step load in its slot, and where 1.0f is loaded); 5 of 350 (f4 / f2 against f2 / f1) |
| `EftRay_DrawRays`, `EftRay_DrawQuad2D` | `eft_s.c` (second part) | 59 of 306: the corner loop counts up in the original (this source gives it with `-fno-rerun-loop-opt`); 311 of 339 (three values in s4..s6 and a frame 0x10 bigger in the original) |
| `EftStreak_Draw`, `EftStreak_Step`, `EftStreak_DrawScreen` | `eft_t_b.c` | 342 of 377 (four spills and fp for a corner address in the original); 96 of 140 (the original computes 1.0f - 0.4f at run time; the compiler folds it here); 356 of 373 (1.0 loaded twice in the original) |
| `EftShotFx_IsOnScreen` | `eft_t_c.c` | 26 of 100: inverse branch layout at the end of each case (as `EftWater_IsOnScreen`); the attempt currently does not compile when enabled |
| `EftPtcl_DrawAxisQuads` | `eft_u_b.c` (second part) | 28 of 460: a `beqzl` with the load of `p->next` in its slot, and one more place |
| `EftLink_InitKeys`, `EftLink_UpdateKeys` | `eft_v_c.c` | 378 of 402 and 225 of 354: register allocation of the key tracks |
| `EftLink_DrawQuad`, `EftLink_InitNode`, `EftLink_DrawBillboard`, `EftLink_Warp`, `EftLink_SetDir` | `eft_w.c` | 26 of 271 and 20 of 439 (registers and scheduling of the packet header constants); 7 of 369 (f20 / f21 swapped); the last two are empty functions whose original still copies the by-value vector to the stack (8 instructions against 3) |
| `EftPart10_UnlinkGroup`, `EftPart10_UnlinkPtcl`, `EftPart10_StartKeys`, `EftPart10_UpdateKeys`, `EftPart10_SetKey` | `eft_x.c` | 3 of 25 each (`next` in a0 in the original, a2 here); same length, register allocation of the shared track addresses (477, 393, 208 instructions) |
| `EftQuad_ListRemove`, `EftQuad_DrawSprite` | `eft_y.c` | 3 of 25 (as `EftPart10_UnlinkGroup`); 26 of 271 (registers of the packet header) |
| `EftLine_DrawSprite`, `EftGndDustSlide_Init` | `eft_z.c` | 26 of 340 (which instruction fills the delay slot of the branch after the projection call); 5 of 63 (a store in the delay slot of a call in the original) |
| `EftGndDustLand_Init`, `EftGndDustImpact_Init` | `eft_z_b.c`, `eft_z_c.c` | 298 of 394 and 68 of 250: the original keeps `&arg->colA / colB` in saved registers and spills copies later |
| `EftGndDust_SpawnBodyDust`, `EftGndDust_DrawPiece`, `EftBlade_AddPoint`, `EftBlade_Draw`, `EftBlade_GetColor`, `EftAnimPart_Draw` | `eft_aa.c` | 21 of 347 (float registers and constant scheduling in the piece loop); 2 of 57 (f15 set before a3); 362 of 385 (which values are spilled); 6 of 389 (two stack slots exchanged); 17 of 31 (the shape of an inlined pointer getter); 16 of 67 (s2 / s3 swapped) |
| `EftOrbTail_DrawStreaks`, `EftOrbTail_InitFrames`, `EftOrbTail_Create` | `eft_ab.c` | 43 of 84 and 76 of 86 (the original keeps one copy of the frame's address per component and does not strength-reduce the loops); 2 of 45 (the order of two stores) |
| `EftRibbon_LoadKey`, `EftRibbon_UpdateKeys`, `EftRibbon_PlaceStrip`, `EftRibbon_PlaceTrail2`, `EftRibbon_DrawStrip` | `eft_ab_c.c` | one instruction short (`arg` copied to v0 at entry); 228 instructions against 233; 60 of 87 and 9 of 59 (the original copies `w` into a1 and reads through the copy); 415 against 414. `PlaceStrip` and `DrawStrip` are compiled as stubs for their callers |
| `EftRibbon_DrawKind1`, `EftZap_Draw`, `EftZap_InitLine`, `EftZap_DrawQuad` | `eft_ab_c.c` (second part) | 15 of 302 (s6 / s7 swapped; compiled as a stub for `EftRibbon_Draw`); 9 of 420 (temporaries of three memsets); 7 of 567 (f2 / f3 swapped in one of three identical blocks); 45 of 274 (packet header constants) |
| `EftMesh_DrawTriClip`, `EftMesh_DrawNowTriClip`, `EftMesh_QueueTri`, `EftMesh_SendTri` | `eft_ad_b.c` | 178 instructions against 180 and 149 against 147 (other induction variables in the fan loop); 644 against 654 (t8 / t9 swapped, store order of the packet headers); 309 against 311 |
| `EftSpr_DrawFlat`, `EftSpr_DrawRot` | `eft_ad_c.c` | 250 instructions against 254; 325 against 407 (the original writes the four corners out one by one) |
| `ColObb_Contact` | `col_a.c` | not matched (1137 of 1140); dead code with no caller; owns the jump table at 0x2F2170 |
| `EftSprAnim_Draw`, `EftSprAnim_DrawQuad`, `EftSprAnim_DrawQuadSubdiv` | `eft_ae.c` | 9 of 110, scheduling only (the original loads the constant 2 into t1 right after reading the draw flags); the quad writers: the original computes the GS context bit as an int after the entry test of the fan loop and uses it unextended / sign-extended for different registers, the two stq pointers swap registers and two header stores swap (one inlined `EftSprAnim_QueueTri` attempt covers both) |
| `BtlAiStep_GuardUntilSafe` | `btl_ai_seq.c` | 2 of 74: `sltiu v0,v0,1` and `li a1,0x10` in the other order around the first branch |
| `FontIcon_PutSprite` | `col_c_b.c` | 125 of 129: same operations; the original builds every 64-bit register value after the visibility test, keeps x0, x1, colour and TEX0 in saved registers and y0, y1 on the stack |
| `BObjChainA_Step` | `bobj_b_b.c` | 9 of 122 (operand order of four address additions around the id stack); owns the 19 constants at 0x2FE718..0x2FE764 (assembly chunk) |
| `Dialog_SetCursor` | `sys/dialog.c` | 7 of 22, registers only: the original has `gDialog` in v1, `choice ^ 1` in v0 and the old cursor in a1 (matches only with the cursor pinned to `$5`, which is not source). Because it is assembly, the two label strings it uses are named objects in dialog.c |
| `HudGauge_UpdateAura` | `hud_b.c` | 351 instructions against 349: the compiler rotates the redraw loop (or, written with a `goto`, keeps `n * 8` in another register); owns the table at 0x2F1B90 |
| `StgPanBlur_UpdateView`, `StgPanBlur_DrawView`, `GfxPost_DrawGlow`, `StgDepthTint_Draw` | `sys/gfxm_a.c` | 60 of 191 (two saved registers, and the original recomputes `view + i * 0x30` for each store); 83 of 454 (registers in the layer loop); 20 of 966 (two texel coordinates stepped as 64-bit values in the up-scaling loops); 51 of 59 (a zero kept in a saved register across a call) |
| `GfxPost_DrawTintRect`, `GfxLens_DrawAll`, `GfxLens_PutCapture`, `GfxWater_DrawView`, `GfxWater_Draw`, `GfxPost_ShiftHighWord` | `sys/gfxm_b.c` | 33 of 136 (the original computes `0 << 4` at run time); 15 of 240 (an indexed store where this walks a pointer; owns the table at 0x2EB6A0); about 230 of 299 (five strip counters against four); about 508 of 640 (nine locals on the stack in the original); one hoisted address (frame 0x80 against 0x90); 87 of 191 (registers only) |
| `TexChain_Build`, `TexChain_BuildPair`, `GfxClut_InitPacket` | `sys/gfxm_b_b.c` | 3 of 94 and 31 of 120 (`addu s0,v0,s0` where the compiler knows the offset is 0x10); 7 of 76 (registers of three constants) |
| `Flash_SkipNamed` | `sys/gfxm_b_c.c` | 16 of 28: where the NULL result is loaded |
| `Num_DrawEx`, `ChrGrid_Build`, `BgmList_ApplyUnlocks` | `view_a_e.c` | 29 of 179 (three saved registers rotated); 210 of 347 (the original keeps three separate copies of the cell copy); 9 of 40 (two registers swapped) |
| `Flash_Advance` | `sys/gfxm_c.c` | one instruction short: the original copies the masked flags before testing bit 0 (`and v1,v1,v0 / move v0,v1 / andi v0,v0,1`), this gives `and / andi v0,v1,1`; about 120 spellings tried |
| `ObjShadow_BuildPacket` | `sys/gfxm_e_b.c` | same code up to register names, but this compiler hoists three header constants out of the batch loop into saved registers (a loop-size threshold: the original body was about eight RTL instructions longer); its 0.02 constant (0x2FC2CC) stays in the assembly `.lit4` chunk |
| `BtlText_PutSprite` | `hud_0_c.c` | 7 of 108: two saved registers exchanged (`x0 + 0x700` in s4 and `u0` in s3 in the original) |
| `HudPrompt_UpdateCue` | `hud_e_b.c` | 14 instructions: the registers of four temporaries (a local-allocation order; 400 declaration orders checked); owns the jump table at 0x2F20E0 and eight `.lit4` words emitted with `LIT4_WORD` |
| `TextBox_DrawClip` | `view_b.c` | one instruction longer: the original copies the clamped colour component with `move v1,v0` (seven times), this narrows it with `andi v1,v0,0xff` (eight times) |
| `ShenScene_StepSeq` | `view_b_d.c` | about 30 instructions around 0x2620D8 ordered differently: the original converts the constant 0.0f of the third blur layer with the full float-to-unsigned sequence on `$f0`, this keeps it in `$f20` and folds the arm; owns the tables at 0x2F33D0..0x2F3450, the jump table at 0x2F3450 and the constants 0x2FE7AC / 0x2FE7B0 |
| `Shen_DrawList`, `Shen_BuildList` | `sys/late_a.c` | same instruction count (229), which of the two registers holding `&ref` each call uses; 4 of 121, two registers that both hold `&work->list` swapped |

Menu overlay (DBZP.BIN, src/menu/), all with a behaviourally exact attempt in `#if 0` above the INCLUDE_ASM:

| Function | File | What differs |
|---|---|---|
| `Train_BuildLists` (0x35A610) | `menu/menu_h_d.c` | 11 of 118 instructions: a folded compare |
| `SimEv34` (0x392190, the shell game) | `menu/menu_u_d.c` | 15 of 551: registers and store order in the block that starts the game |
| `Option_Draw` (0x3A3848) | `menu/menu_x_c.c` | 14 of 2153: register choices only; ten of its clip-name strings are shared with later C functions and live in its .s file meanwhile (`sOptClip*`, `force_migration`) |
| `Dc_Main` (0x3A9850) | `menu/menu_z_b.c` | flow identical; see the note in the source |
| `DcMenu_DrawPlates` (0x3A9BC8) | `menu/menu_z_c.c` | see the note in the source |
| `DcPass_WrapPos`, `DcPass_Decode`, `DcPass_DrawStatus` | `menu/menu_z_d.c` | see the notes in the source |
| `DcPass_DrawRows` (0x3AC858) | `menu/menu_z_d.c` | 47 of 161: saved registers assigned differently |

`StgVu_RotateZ`, `StgVu_RotateX`, `StgVu_RotateY` (0x240C68..0x240DB8) are hand-written VU0 macro code and stay an
assembly chunk between `stg_a.c` and `stg_a_b.c`. They cannot be INCLUDE_ASM: the per-function files splat writes
for a C segment spell the VU0 registers `ACC` / `Q` without `$`, which the assembler rejects. The vector library
(`src/sys/vu0_a_c*.c`, `vu0_b_c.c`) carries its hand-written routines as top-level `__asm__` blocks instead; the
same could be done here.

- Why several overlay modules' uninitialised globals overlap at 0x31EA80..0x31EAA4 in the main executable's
  `.bss` (CharRef's state table against UbMenu / sim words): kept as fixed-address symbols.
- Whether `gBracket` (0x3B5918) was defined by the Bracket screen file (menu_k_b.c, as linked) or elsewhere: any
  object between TourMenu and TourBg in link order gives the same image.

## Game structure

- What each of the 70 `gProgress->mode` values (overlay dispatch) is.
- Which game modes battle modes 2..5 correspond to in the menus; where modes 8 and 9 come from.
- The meaning of most battle event ids, of "character flag 7" and character flag 0x128.
- The save block's `slot[9]`, `unlockFlags`, `rule[6]`, and its large unidentified ranges.
- What sets the per-side voice mute (`func_00259E20`).
- Whether further code is ever loaded from the archives (only one overlay table entry exists).

## Not started

- The 3D renderer, the model/texture/animation formats and the nine VU1 microprograms.
- `SOUNDS.IRX` (the sound driver) and the bank format.
- The archives: nothing has been extracted or catalogued.
- The Wii build's netcode.
