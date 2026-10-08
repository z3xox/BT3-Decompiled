# Readable names for the C files

> **Applied on the branch `rename`, 2026-10-08** (local; the port repository has not followed yet). First 13 merges
> of neighbouring files, each kept only with the build byte-identical, then 198 renames in one step; both outputs
> still equal the disc's. The list that was applied is `docs/file_rename_map.txt` (`old new` per line, the names
> after the merges); `scripts/rename_files.py` applies such a list and `scripts/try_merge.py` tries a merge.
> What follows is the draft as it was written, and at the end the merge trials and what is left.


A proposal only (2026-10-08, second draft: plain words written out, the game's and the code's own short forms kept). Nothing here is applied. Written at the user's request to read and change.

210 of the 273 C files would get a new name; 63 keep theirs.

## How to read it

- The names follow the functions inside: every function already has a real name with a module prefix (`HudGauge_`, `EftRibbon_`, `CharSel_`), and a file is named for the module it holds.
- **clear**: one module, or one that is most of the file. **mixed**: several modules share the file and it is named for the largest; a better name may need the file to be split, which is a separate question. **slice**: one part of a larger original source file whose other parts are separate files here.
- "functions" counts the definitions and the ones still in assembly. "modules" are the three most common prefixes with their counts.
- A rename changes nothing in the built game. Each one also touches the file's header in include/, its line in the two build lists (config/*.yaml), its symbol list in config/symbols/, the `#include` lines elsewhere and the docs. The port repository has its own copy of these files and must follow in the same step.

## Words

Plain words are written out where the name stays short (`char_select`, `mission_result`, `password_window`,
`stg_collision`). Two kinds of short forms are kept, because the functions inside carry them as their prefix:

| kept | stands for |
|---|---|
| `btl_` | battle; in most files, the fighter |
| `eft_` | effect |
| `stg_` | stage |
| `col_` | collision |
| `hud_` | the fight's on-screen display |
| `obj_`, `btl_obj_` | battle object (an animated model) |
| `ub_`, `ubz_` | Ultimate Battle, Ultimate Battle Z |
| `sim_` | Sim Dragon |
| `dc_` | Data Center |
| `evo_` | Evolution Z |
| `history_`, `tour_` | Dragon History, Dragon World Tour |
| `shen_` | the dragon (Shenron) and its wish screen |
| `char_` | character |

## Open choices

1. **A folder `src/ui/`** for the menu support code that lives in the main executable (windows, text box, character viewer, the dragon screen). Today it is spread over `battle/view_*` and `sys/late_*`. Without it those files would stay in `battle/` and `sys/` under their new names.
2. **Parts of one module** are `name`, `name_b` (`eft_quad`, `eft_quad_b`). The alternative is `_1`, `_2`.
3. **The effect files** (`eft_*`, 57 of them) have no better grouping than their module; several are mixed.
4. **File name and function prefix differ** where a word was written out (`char_select.c` holds `CharSel_` functions, `password_window.c` holds `PassWin_`). Renaming the functions to match would be a second, larger step, and is not proposed here.

## System (src/sys)

| now | proposed | functions | modules | kind | note |
|---|---|---|---|---|---|
| `sys/align` | `sys/mem_align` | 18 | Mem 18 | clear |  |
| `sys/gfxm_a` | `sys/gfx_post` | 47 | GfxPost 13, StgPanBlur 8, StgGlare 8 | mixed |  |
| `sys/gfxm_b` | `sys/gfx_screen` | 34 | GfxLens 13, GfxWater 7, GfxPost 5 | mixed |  |
| `sys/gfxm_b_b` | `sys/tex_file` | 18 | TexChain 6, TexFile 6, Tex 4 | mixed |  |
| `sys/gfxm_b_c` | `sys/flash_read` | 12 | Flash 12 | clear |  |
| `sys/gfxm_c` | `sys/flash` | 76 | Flash 29, FlashTl 14, FlashClipList 9 | mixed |  |
| `sys/gfxm_d` | `sys/flash_clip` | 14 | FlashClipList 14 | clear |  |
| `sys/gfxm_d_b` | `battle/obj_draw` | 6 | BtlObjDraw 6 | clear |  |
| `sys/gfxm_d_c` | `battle/obj_render` | 19 | ObjDraw 7, ObjGs 6, BtlObjDraw 4 | mixed |  |
| `sys/gfxm_e` | `battle/obj_gs_env` | 5 | ObjGs 5 | clear |  |
| `sys/gfxm_e_b` | `battle/obj_shadow` | 29 | ObjShadow 21, BtlObjMdl 7 | clear |  |
| `sys/gfxm_e_c` | `battle/model_tex` | 1 | MdlTex 1 | clear |  |
| `sys/gfxm_e_d` | `battle/stg_reloc` | 2 | StgOctree 1, BtlStage 1 | mixed |  |
| `sys/late_a` | `ui/shen_wish` | 33 | Shen 25, ShenList 6 | clear | main-executable menu screens: a new folder ui/ for them and for view_* |
| `sys/late_a_b` | `ui/shen_confirm` | 14 | ShenCfm 14 | clear |  |
| `sys/late_a_c` | `ui/shen_save` | 8 | ShenSave 8 | clear |  |
| `sys/mcflow_a` | `sys/memcard_flow` | 22 | McFlow 22 | clear |  |
| `sys/misc_a` | `sys/password_old` | 21 | OldPass 21 | clear |  |
| `sys/misc_a_b` | `sys/password_chara` | 18 | ChrPass 18 | clear |  |
| `sys/randf` | `sys/rand_util` | 5 | Rand 5 | clear |  |
| `sys/vu0_a_c` | `sys/int_vec` | 18 | IVec4 9, IVec3 8, IVec 1 | mixed |  |
| `sys/vu0_a_c_b` | `sys/matrix_screen` | 1 | Mtx 1 | clear |  |
| `sys/vu0_a_c_c` | `sys/matrix_build` | 15 | Mtx 8, Vu0Cur 7 | mixed |  |
| `sys/vu0_b_c` | `sys/vu0_math` | 25 | Vu0 11, Vec3 6, Vu0Cur 3 | mixed |  |

## Battle (src/battle)

| now | proposed | functions | modules | kind | note |
|---|---|---|---|---|---|
| `battle/bobj_a` | `battle/btl_obj_anim` | 97 | BtlObj 32, BtlObjAnim 23, BtlObjBody 10 | mixed |  |
| `battle/bobj_b_b` | `battle/btl_obj_chain` | 15 | BtlObj 7, BObjChainB 4, BObjChainA 4 | mixed |  |
| `battle/btl_act_a` | `battle/btl_act_1` | 44 | BtlAct 39, BtlActB 5 | slice | slice of one original file of action handlers; a number says no more than is known |
| `battle/btl_act_c` | `battle/btl_act_2` | 30 | BtlAct 30 | slice | slice, as above |
| `battle/btl_act_d` | `battle/btl_act_3` | 18 | BtlAct 18 | slice | slice, as above |
| `battle/btl_act_e` | `battle/btl_act_4` | 20 | BtlAct 20 | slice | slice: actions 0x1A..0x35 and 0xFA..0xFC |
| `battle/btl_act_f` | `battle/btl_act_super` | 40 | BtlSuper 19, BtlAct 19, BtlActThrow 2 | mixed |  |
| `battle/btl_act_h` | `battle/btl_act_grab` | 2 | BtlActThrow 1, BtlAct 1 | mixed |  |
| `battle/btl_act_h_b` | `battle/btl_act_change` | 37 | BtlAct 22, BtlSkill 5, BtlDecide 4 | mixed |  |
| `battle/btl_act_j` | `battle/btl_act_decide` | 28 | BtlAct 28 | clear |  |
| `battle/btl_ai_act` | `battle/btl_ai_sense` | 56 | BtlAiSense 33, BtlAiAtk 7, BtlAiPad 5 | mixed |  |
| `battle/btl_ai_cond` | `battle/btl_ai_think` | 85 | BtlAiCond 37, AiThink 28, AiThCond 15 | mixed |  |
| `battle/btl_capi_a` | `battle/btl_char_api_a` | 81 | BtlCharApi 81 | clear | the three fighter-interface files are one run of accessors in address order |
| `battle/btl_capi_b` | `battle/btl_char_api_c` | 136 | BtlCharApi 66, BtlCtrl 46, BtlSide 18 | mixed |  |
| `battle/btl_char_api` | `battle/btl_char_api_b` | 58 | BtlCharApi 58 | clear |  |
| `battle/btl_char_coll_b` | `battle/btl_hit_reaction` | 25 | BtlColl 25 | clear |  |
| `battle/btl_char_ctl_b` | `battle/btl_change` | 14 | BtlChange 13, BtlChars 1 | clear |  |
| `battle/btl_char_ctl_c` | `battle/btl_head_tracking` | 6 | BtlChar 6 | clear |  |
| `battle/btl_char_ctl_d` | `battle/btl_char_pose` | 28 | BtlChar 26, BtlChars 2 | clear |  |
| `battle/btl_char_flag_clash` | `battle/btl_clash` | 15 | BtlClash 15 | clear |  |
| `battle/btl_char_flag_opp` | `battle/btl_opponent` | 37 | BtlOpp 37 | clear |  |
| `battle/btl_char_flag_snd` | `battle/btl_char_sound` | 23 | BtlCharSnd 23 | clear |  |
| `battle/btl_char_fx_c` | `battle/btl_char_fx_b` | 39 | BtlFx 18, BtlPartner 15, BtlFxObj 4 | mixed |  |
| `battle/btl_char_get` | `battle/btl_char_util` | 39 | BtlChar 26, BtlUtil 13 | clear |  |
| `battle/btl_char_status` | `battle/btl_stats` | 37 | BtlStat 37 | clear |  |
| `battle/btl_char_status_anim` | `battle/btl_anim` | 42 | BtlAnim 42 | clear |  |
| `battle/btl_tech_a` | `battle/btl_param` | 127 | BtlParam 51, BtlAtk 46, BtlCtrl 30 | mixed |  |
| `battle/btl_tech_b` | `battle/btl_tech` | 182 | BtlKiBlast 66, BtlSuper 51, BtlSkill 34 | mixed |  |
| `battle/col_a` | `battle/col_box` | 27 | ColBox 11, ColMesh 7, ColObb 4 | mixed |  |
| `battle/col_b` | `battle/col_primitives` | 42 | ColSeg 10, ColSphere 10, ColBounds 5 | mixed |  |
| `battle/col_c` | `sys/font` | 101 | Font 101 | clear | not collision: the text printer; moves to sys/ |
| `battle/col_c_b` | `sys/font_icon` | 28 | FontIcon 20, FontTag 8 | clear |  |
| `battle/eft_a` | `battle/eft_core` | 91 | EftHit 48, EftHitArena 10, EftCam 10 | mixed |  |
| `battle/eft_aa` | `battle/eft_char_parts` | 84 | EftAnimPart 17, EftCharSlot 15, EftBlade 13 | mixed | grab-bag of small per-character parts |
| `battle/eft_ab` | `battle/eft_orb_tail` | 35 | EftOrbTail 32, EftOrbTailMgr 3 | clear |  |
| `battle/eft_ab_b` | `battle/eft_transform` | 17 | EftTransform 14, EftTransformMgr 3 | clear |  |
| `battle/eft_ab_c` | `battle/eft_ribbon` | 60 | EftRibbon 35, EftZap 12, EftZapMgr 5 | clear |  |
| `battle/eft_ad` | `battle/eft_zap` | 29 | EftZap 19, EftShock 7, EftShockMgr 3 | clear |  |
| `battle/eft_ad_b` | `battle/eft_mesh` | 36 | EftMesh 26, EftObj 10 | clear |  |
| `battle/eft_ad_c` | `battle/eft_sprite` | 2 | EftSpr 2 | clear |  |
| `battle/eft_ae` | `battle/eft_sprite_anim` | 93 | EftSprAnim 31, BtlTask 17, BtlTaskList 14 | mixed | also holds the battle task tree (BtlTask, 31 functions): could be split in two names only if the file is split |
| `battle/eft_b` | `battle/eft_stage_a` | 49 | EftBubble 23, EftGeyser 13, EftStageScroll 8 | mixed |  |
| `battle/eft_c` | `battle/eft_stage_b` | 77 | EftSurf 25, EftWeather 23, EftStage 15 | mixed |  |
| `battle/eft_d` | `battle/eft_surface_out` | 12 | EftSurf 12 | clear |  |
| `battle/eft_d_b` | `battle/eft_burst` | 39 | EftBurst 33, EftBurstLayer 3, Eft 2 | clear |  |
| `battle/eft_det_a` | `battle/eft_detect` | 40 | EftDet 18, EftVolleyAim 8, StgGround 5 | mixed |  |
| `battle/eft_det_b` | `battle/stg_collision` | 33 | StgCol 25, StgShadow 8 | clear |  |
| `battle/eft_det_b_b` | `battle/stg_nav` | 12 | StgNav 11, StgNavNode 1 | clear |  |
| `battle/eft_det_b_c` | `battle/btl_ai_seq_head` | 2 | BtlAiSeq 2 | clear | two functions that belong to btl_ai_seq.c but cannot be merged into it (the merge changes the code) |
| `battle/eft_e` | `battle/eft_water` | 80 | EftWater 24, EftSteam 17, EftWaterTrail 7 | mixed |  |
| `battle/eft_g` | `battle/eft_shot` | 77 | EftShot 17, EftStorm 13, EftBound 11 | mixed |  |
| `battle/eft_h` | `battle/eft_emit` | 64 | EftEmit 28, EftVolley 12, EftShot 9 | mixed |  |
| `battle/eft_i` | `battle/eft_sweep` | 43 | EftEmit 20, EftSweep 12, EftFollow 7 | mixed |  |
| `battle/eft_j` | `battle/eft_shot_tech` | 65 | EftShotTech 14, EftPropShot 13, EftMulti 11 | mixed |  |
| `battle/eft_k` | `battle/eft_obj_tech` | 81 | EftObjTech 18, EftRushShot 17, EftShotTech 14 | mixed |  |
| `battle/eft_l_b` | `battle/eft_ring_shot` | 20 | EftRingShot 16, EftRingShotMgr 4 | clear |  |
| `battle/eft_l_c` | `battle/eft_absorb` | 13 | EftAbsorb 10, EftAbsorbMgr 3 | clear |  |
| `battle/eft_l_d` | `battle/eft_speed_line_spawn` | 2 | EftSpdLine 2 | clear |  |
| `battle/eft_m` | `battle/eft_aura` | 50 | EftAura 30, EftSpdLine 20 | clear |  |
| `battle/eft_n` | `battle/eft_aura_b` | 53 | EftAura 29, EftBolt 12, EftAuraTask 6 | mixed |  |
| `battle/eft_o` | `battle/eft_rays` | 39 | EftRays 22, EftBoltTask 6, EftBolt 4 | mixed |  |
| `battle/eft_o_b` | `battle/eft_blast_obj` | 32 | EftBlastObj 28, EftBlastObjMgr 4 | clear |  |
| `battle/eft_o_c` | `battle/eft_body_fx` | 18 | EftBodyFx 9, EftDiscMgr 4, EftBodyFxMgr 3 | mixed |  |
| `battle/eft_p` | `battle/eft_disc` | 30 | EftDisc 27, EftDiscMgr 3 | clear |  |
| `battle/eft_p_b` | `battle/eft_glow` | 23 | EftGlow 22, EftPOt 1 | clear |  |
| `battle/eft_q` | `battle/eft_trail` | 101 | EftTrail 15, EftGlow 12, EftRushBurst 11 | mixed | grab-bag of four modules; named for the first |
| `battle/eft_r` | `battle/eft_struggle` | 105 | EftStruggle 18, EftKiBomb 14, EftClashSpark 13 | mixed | grab-bag (beam struggle, ki bomb, clash sparks, ki blasts); named for the first |
| `battle/eft_s` | `battle/eft_chain` | 78 | EftChain 35, EftRay 20, EftKiObj 11 | mixed |  |
| `battle/eft_t_b` | `battle/eft_streak` | 27 | EftStreak 20, EftStreakMgr 4, EftTOt 1 | clear |  |
| `battle/eft_t_c` | `battle/eft_shot_fx` | 20 | EftShotFx 19, EftShotFxMgr 1 | clear |  |
| `battle/eft_u_b` | `battle/eft_particle` | 40 | EftPtcl 36, EftPtclMgr 4 | clear |  |
| `battle/eft_v_b` | `battle/eft_impact` | 16 | EftImpact 12, EftImpactMgr 4 | clear |  |
| `battle/eft_v_c` | `battle/eft_link` | 15 | EftLink 11, EftLinkMgr 4 | clear |  |
| `battle/eft_w` | `battle/eft_link_b` | 44 | EftLink 31, EftPart10 9, EftPart10Mgr 4 | clear |  |
| `battle/eft_x` | `battle/eft_part10` | 34 | EftPart10 34 | clear |  |
| `battle/eft_x_b` | `battle/eft_layer3` | 4 | EftLayer3 4 | clear |  |
| `battle/eft_x_c` | `battle/eft_quad` | 12 | EftQuad 8, EftQuadMgr 4 | clear |  |
| `battle/eft_y` | `battle/eft_quad_b` | 35 | EftQuad 35 | clear |  |
| `battle/eft_z` | `battle/eft_billboard` | 65 | EftBill 20, EftGndDust 11, EftLine 6 | mixed |  |
| `battle/eft_z_b` | `battle/eft_dust_land` | 5 | EftGndDustLand 5 | clear |  |
| `battle/eft_z_c` | `battle/eft_dust_impact` | 9 | EftGndDustImpact 5, EftGndDust 4 | mixed |  |
| `battle/hud_0` | `battle/pause_menu` | 12 | PauseMenu 9, BtlGame 3 | clear |  |
| `battle/hud_0_b` | `battle/btl_menu` | 45 | BtlMenu 45 | clear |  |
| `battle/hud_0_c` | `battle/btl_text_head` | 7 | BtlText 7 | clear |  |
| `battle/hud_a` | `battle/hud` | 17 | Hud 15, HudNode 2 | clear |  |
| `battle/hud_a_b` | `battle/hud_team` | 29 | HudTeam 29 | clear |  |
| `battle/hud_a_c` | `battle/hud_caption` | 5 | HudCaption 5 | clear |  |
| `battle/hud_a_d` | `battle/hud_gauge_head` | 5 | HudGauge 5 | clear |  |
| `battle/hud_b` | `battle/hud_gauge` | 33 | HudGauge 32 | clear |  |
| `battle/hud_c` | `battle/hud_gauge_tail` | 2 | HudGauge 2 | clear |  |
| `battle/hud_c_b` | `battle/hud_combo` | 21 | HudCombo 21 | clear |  |
| `battle/hud_c_c` | `battle/hud_sprite` | 23 | HudSprite 18, HudGfx 4, HudRes 1 | clear |  |
| `battle/hud_d` | `battle/hud_node` | 6 | HudNode 5, HudSprite 1 | clear |  |
| `battle/hud_d_b` | `battle/hud_notice` | 22 | HudNotice 22 | clear |  |
| `battle/hud_e` | `battle/hud_notice_b` | 8 | HudNotice 8 | clear |  |
| `battle/hud_e_b` | `battle/hud_prompt` | 28 | HudPrompt 27 | clear |  |
| `battle/hud_e_c` | `battle/hud_timer` | 12 | HudTimer 12 | clear |  |
| `battle/hud_e_d` | `battle/btl_pause` | 10 | BtlPause 10 | clear |  |
| `battle/hud_e_e` | `battle/stg_rigid_list` | 2 | StgRigidList 1, StgRigid 1 | mixed |  |
| `battle/stg_a` | `battle/stg` | 35 | BtlStage 12, StgDbg 11, StgOfsTable 3 | mixed |  |
| `battle/stg_a_b` | `battle/stg_parts` | 32 | BtlStage 25, StgObj 4, Stg 2 | clear |  |
| `battle/stg_b` | `battle/stg_ambient` | 89 | StgAmb 33, BtlStage 18, ScrWarp 17 | mixed |  |
| `battle/stg_c` | `battle/screen_fx` | 69 | StgHaze 22, StgBlur 21, StgTint 12 | mixed |  |
| `battle/stg_d` | `battle/stg_rigid` | 20 | StgRigid 12, StgRigidList 7, StgAabb 1 | clear |  |
| `battle/stg_d_b` | `battle/stg_model_anim` | 5 | StgModel 5 | clear |  |
| `battle/stgm_a` | `battle/stg_model_draw` | 13 | StgModel 13 | clear |  |
| `battle/stgm_a_b` | `sys/memcard` | 20 | McCard 19, McCardFile 1 | clear | not stage code: memory card operations; moves to sys/ |
| `battle/view_a` | `ui/reward_window` | 8 | GetWin 8 | clear |  |
| `battle/view_a_b` | `ui/message_window` | 8 | MsgWin 8 | clear |  |
| `battle/view_a_c` | `ui/icon_window` | 6 | IconWin 6 | clear |  |
| `battle/view_a_d` | `ui/char_viewer` | 19 | ChrView 19 | clear |  |
| `battle/view_a_e` | `ui/menu_util` | 40 | ChrGrid 9, Num 7, StgGrid 7 | mixed | Progress_Init, the character and stage grids, number drawing, the head of the text box |
| `battle/view_b` | `ui/text_box` | 10 | TextBox 9 | clear |  |
| `battle/view_b_b` | `ui/char_table` | 11 | ChrTbl 5, ItemSet 4, ItemTbl 2 | mixed |  |
| `battle/view_b_c` | `ui/menu_util_b` | 15 | LipSync 4, Voice 2, CpuLevel 2 | mixed |  |
| `battle/view_b_d` | `ui/shen_scene` | 17 | ShenScene 16 | clear |  |
| `battle/view_b_e` | `sys/debug_stubs` | 20 | Dbg 20 | clear |  |

## Menu overlay (src/menu)

| now | proposed | functions | modules | kind | note |
|---|---|---|---|---|---|
| `menu/menu_a` | `menu/main_menu` | 8 | MainMenu 8 | clear |  |
| `menu/menu_a_b` | `menu/progress` | 1 | Progress 1 | clear |  |
| `menu/menu_a_c` | `menu/title` | 10 | Title 10 | clear |  |
| `menu/menu_a_d` | `menu/mode_menu` | 11 | ModeMenu 11 | clear |  |
| `menu/menu_b_b` | `menu/mode_background` | 3 | ModeBg 3 | clear |  |
| `menu/menu_b_c` | `menu/history_outro` | 9 | HistOutro 8, Hist 1 | clear |  |
| `menu/menu_b_d` | `menu/history_select` | 11 | HistSel 11 | clear |  |
| `menu/menu_c_b` | `menu/history_guide` | 2 | HistGuide 2 | clear |  |
| `menu/menu_c_c` | `menu/history_result` | 11 | HistResult 11 | clear |  |
| `menu/menu_c_d` | `menu/history_save` | 7 | HistSave 7 | clear |  |
| `menu/menu_c_e` | `menu/char_select` | 16 | CharSel 16 | clear |  |
| `menu/menu_e_b` | `menu/team_select` | 21 | TeamSel 21 | clear |  |
| `menu/menu_g` | `menu/item_panel` | 9 | ItemPanel 9 | clear |  |
| `menu/menu_g_b` | `menu/duel` | 2 | Duel 2 | clear |  |
| `menu/menu_g_c` | `menu/duel_menu` | 12 | DuelMenu 12 | clear |  |
| `menu/menu_h_b` | `menu/char_reference` | 20 | CharRef 20 | clear |  |
| `menu/menu_h_c` | `menu/reference_training_mode` | 2 | CharRefMode 1, TrainMode 1 | mixed |  |
| `menu/menu_h_d` | `menu/training` | 47 | Train 46 | clear |  |
| `menu/menu_i_b` | `menu/boot_card` | 4 | BootCard 4 | clear |  |
| `menu/menu_i_c` | `menu/logo` | 7 | Logo 4, FirstRun 3 | mixed |  |
| `menu/menu_i_d` | `menu/entry_select` | 14 | EntrySel 13, Tour 1 | clear |  |
| `menu/menu_j_b` | `menu/tour_menu` | 9 | TourMenu 9 | clear |  |
| `menu/menu_k_b` | `menu/bracket` | 7 | Bracket 7 | clear |  |
| `menu/menu_k_c` | `menu/bracket_guide` | 2 | Bracket 2 | clear |  |
| `menu/menu_k_d` | `menu/tour_background` | 3 | TourBg 3 | clear |  |
| `menu/menu_k_e` | `menu/bracket_clips` | 1 | Bracket 1 | clear |  |
| `menu/menu_k_f` | `menu/bracket_logic` | 21 | Bracket 21 | clear |  |
| `menu/menu_l_b` | `menu/solo_select` | 12 | SoloSel 12 | clear |  |
| `menu/menu_m_b` | `menu/ub_team_select` | 17 | UbTeamSel 17 | clear |  |
| `menu/menu_n_b` | `menu/ubz_select` | 9 | UbzSel 9 | clear |  |
| `menu/menu_n_c` | `menu/ub_rank` | 14 | UbRank 14 | clear |  |
| `menu/menu_n_d` | `menu/ub_result` | 8 | UbResult 8 | clear |  |
| `menu/menu_o_b` | `menu/disc_fusion` | 11 | DiscFusion 11 | clear |  |
| `menu/menu_o_c` | `menu/ub` | 4 | Ub 4 | clear |  |
| `menu/menu_o_d` | `menu/mission_select` | 10 | MisSel 10 | clear |  |
| `menu/menu_p_b` | `menu/mission_result` | 10 | MisResult 10 | clear |  |
| `menu/menu_p_c` | `menu/ub_menu` | 10 | UbMenu 10 | clear |  |
| `menu/menu_p_d` | `menu/ub_score` | 13 | UbScore 13 | clear |  |
| `menu/menu_q_b` | `menu/sim_day` | 30 | SimDay 30 | clear |  |
| `menu/menu_r_b` | `menu/sim_event` | 1 | SimEvent 1 | clear |  |
| `menu/menu_r_c` | `menu/sim_result` | 8 | SimResult 8 | clear |  |
| `menu/menu_r_d` | `menu/sim_top` | 9 | SimTop 9 | clear |  |
| `menu/menu_s_b` | `menu/survival_select` | 9 | SurvSel 9 | clear |  |
| `menu/menu_s_c` | `menu/survival_result` | 7 | SurvResult 7 | clear |  |
| `menu/menu_s_d` | `menu/sim_event_a` | 31 | SimEv00 1, SimEv01 1, SimEv02 1 | mixed |  |
| `menu/menu_t_b` | `menu/sim_event_card` | 7 | SimEv28 4 | clear |  |
| `menu/menu_u` | `menu/sim_event_b` | 1 |  | clear | check before naming: the stem covers eight source files and this one holds one function |
| `menu/menu_u_b` | `menu/sim_event_rock` | 2 | SimRock 1 | clear |  |
| `menu/menu_u_c` | `menu/sim_event_33` | 1 |  | clear |  |
| `menu/menu_u_d` | `menu/sim_event_shell` | 4 | SimPopo 3 | clear |  |
| `menu/menu_u_e` | `menu/sim_event_35` | 1 |  | clear |  |
| `menu/menu_u_f` | `menu/sim_event_36` | 1 |  | clear |  |
| `menu/menu_u_g` | `menu/evo_z` | 5 | EvoZ 5 | clear |  |
| `menu/menu_u_h` | `menu/evo_z_clips` | 4 | EvoZ 4 | clear |  |
| `menu/menu_v_b` | `menu/evo_z_b` | 13 | EvoZ 13 | clear |  |
| `menu/menu_v_c` | `menu/item_help` | 5 | ItemHelp 5 | clear |  |
| `menu/menu_v_d` | `menu/shop` | 16 | Shop 16 | clear |  |
| `menu/menu_w_b` | `menu/evo_mode` | 1 | EvoMode 1 | clear |  |
| `menu/menu_w_c` | `menu/evo_top` | 20 | EvoTop 20 | clear |  |
| `menu/menu_x_b` | `menu/option_mode` | 1 | OptMode 1 | clear |  |
| `menu/menu_x_c` | `menu/option` | 14 | Option 14 | clear |  |
| `menu/menu_y_b` | `menu/dc_list` | 67 | DcList 42, DcView 17, DcChars 5 | clear |  |
| `menu/menu_z_b` | `menu/dc` | 1 | Dc 1 | clear |  |
| `menu/menu_z_c` | `menu/dc_menu` | 22 | DcMenu 22 | clear |  |
| `menu/menu_z_d` | `menu/dc_password` | 68 | DcPass 60, DcPassText 5, DcPassList 1 | clear |  |
| `menu/menu_za_b` | `menu/password_window` | 10 | PassWin 10 | clear |  |
| `menu/menu_za_c` | `menu/password_check` | 6 | PassChk 6 | clear |  |
| `menu/menu_za_d` | `menu/replay_menu` | 36 | ReplayMenu 36 | clear |  |
| `menu/menu_za_e` | `menu/dc_save` | 8 | DcSave 8 | clear |  |

## Kept as they are

`sys/adx`, `sys/bpe`, `sys/color_fade`, `sys/common`, `sys/debug`, `sys/dialog`, `sys/dma`, `sys/fade`, `sys/file`, `sys/game_pad`, `sys/gfx`, `sys/gfx_ot`, `sys/gsc`, `sys/heap`, `sys/heap_info`, `sys/iop_heap`, `sys/job`, `sys/list`, `sys/loading`, `sys/math3d`, `sys/mathf`, `sys/mem_read`, `sys/movie`, `sys/pad`, `sys/pad_watch`, `sys/queue`, `sys/ramp`, `sys/rand`, `sys/rigid`, `sys/save`, `sys/snd`, `sys/sprite`, `sys/spu_heap`, `sys/timer`, `sys/vu1_packet`, `battle/battle`, `battle/battle_load`, `battle/battle_work`, `battle/btl_ai_mgr`, `battle/btl_ai_seq`, `battle/btl_cam`, `battle/btl_char_action`, `battle/btl_char_cam`, `battle/btl_char_cam_cut`, `battle/btl_char_cam_modes`, `battle/btl_char_flag`, `battle/btl_char_fx`, `battle/btl_char_hit`, `battle/btl_char_member`, `battle/btl_char_mgr`, `battle/btl_char_move`, `battle/btl_demo_cam`, `battle/btl_facade`, `battle/btl_input`, `battle/btl_obj`, `battle/btl_pool`, `battle/btl_replay`, `battle/btl_scene`, `battle/btl_script`, `battle/btl_script_cmd`, `battle/btl_seq`, `battle/orbit_cam`, `cri/adxt`

## Which neighbouring files can be one file (trial of 2026-10-08)

`scripts/try_merge.py A B` appends B to A, gives A B's data in the build lists, drops declarations the two
parts make twice, builds, compares both outputs with the disc's, and restores the tree. Run over 49 pairs of
neighbouring files of one module. Nothing was kept.

**Merged and still byte-identical (18 pairs, verified):**

| files | would be |
|---|---|
| `sys/gfxm_b_c` + `gfxm_c` + `gfxm_d` (two pairs) | one `flash` file |
| `sys/vu0_a_c` + `vu0_a_c_b` + `vu0_a_c_c` (two pairs) | one file of the vector library's first half |
| `battle/btl_char_cam` + `btl_char_cam_cut` + `btl_char_cam_modes` (two pairs) | one fighter camera file |
| `battle/btl_char_ctl_b` + `_c` + `_d` (two pairs) | adjacent, but three different things (change requests, head tracking, pose) |
| `battle/eft_z` + `eft_z_b` + `eft_z_c` (two pairs) | one file with all of the ground dust |
| `battle/eft_det_b` + `eft_det_b_b` | stage collision and navigation |
| `battle/eft_det_b_c` + `btl_ai_seq` | the AI sequence file with its two head functions (the note that this merge changes the code did not hold in this trial) |
| `battle/btl_act_h` + `btl_act_h_b` | the grab handler with the handlers after it |
| `battle/hud_0` + `hud_0_b` | the pause menu |
| `battle/hud_0_c` + `btl_seq` | the skill-list text with its head |
| `battle/hud_c_c` + `hud_d` | the HUD's sprite and node library |
| `battle/hud_e_e` + `stg_d` | the rigid bodies with their two list helpers |
| `battle/view_a_e` + `view_b` | the text box in one piece |

Each pair was tried alone; a chain of three is two verified pairs, not a verified triple.

**Built, but bytes changed (5 pairs; the cause was not looked for):** `sys/gfxm_e` + `gfxm_e_b` (1 byte),
`battle/bobj_a` + `bobj_b_b` (2 bytes), `sys/late_a` + `late_a_b` (409), `sys/misc_a` + `misc_a_b`, `battle/col_a` +
`col_b`. The large differences look like data that moved (the script puts the second file's data straight after
the first's), not like different code; the small ones may be real. Not settled.

**Not answered (26 pairs):** the merged file did not build. Most are the script's limits, not findings: the two
parts describe a shared structure or variable differently and a person has to choose (`eft_c` + `eft_d`, `eft_w` +
`eft_x`, `hud_a_d` + `hud_b`, the action handler slices, the fighter interface files); the script dropped a
variable's declaration it should have kept (`eft_x_c` + `eft_y`, `eft_m` + `eft_n`, `hud_b` + `hud_c` and others);
the overlay's build list is not rewritten correctly (the three menu pairs); and `stg_a` + `stg_a_b` have a
hand-written assembly chunk between them, which is a real reason to stay two files.

## What was applied, and what is left (2026-10-08)

**Merged** (the first file's name, then renamed): the movie player (`gfxm_b_c` + `gfxm_c` + `gfxm_d` -> `sys/flash`);
the fighter camera (three files -> `btl_char_cam`); the ground dust (`eft_z` + `_b` + `_c` -> `eft_ground_dust`); the
HUD's sprites and nodes (-> `hud_sprite`); the menu helpers with the text box (-> `ui/menu_util_1`); the skill-list
text with the battle sequence (-> `btl_seq`); the rigid bodies (-> `stg_rigid`); the grab handler with the handlers
after it (-> `btl_act_change`); the AI sequence with its two head functions (-> `btl_ai_seq`); the text printer with
its icon tags (-> `sys/font`); the aura (`eft_m` + `eft_n` -> `eft_aura`); three slices of action handlers (`btl_act_c`
+ `_d` + `_e` -> `btl_act_2`); the fighter interface's last two files (-> `btl_char_api_2`).

**Tried and must stay two files** (the merged file builds, the code comes out different):

| files | what changes |
|---|---|
| `eft_quad_1` + `eft_quad_2` | code 8 bytes longer |
| `hud_gauge_2` + `hud_gauge_3` | code 8 bytes longer |
| `obj_gs_env` + `obj_shadow` | code 4 bytes longer |
| `btl_obj_anim` + `btl_obj_chain` | 2 bytes differ in place |
| `btl_char_api_1` + `btl_char_api_2` | 1 byte differs in place |
| `eft_mesh` + `eft_sprite` | code 4 bytes longer |
| `stg_model_anim` + `stg_model_draw` | code 24 bytes longer |
| `bracket` + `bracket_guide` | code 4 bytes longer, read-only data 4 shorter |
| `shen_wish` + `shen_confirm`, `password_old` + `password_chara`, `evo_z_1` + `evo_z_2`, `col_box` + `col_primitives` | data sizes change (strings or constants the two files both hold are stored once) |

**Merged cleanly in the trial but left apart, because they are different things:** the fighter control files
(`btl_change`, `btl_head_tracking`, `btl_char_pose`), `stg_collision` + `stg_nav`, `pause_menu` + `btl_menu`, the
vector library's first half (`int_vec`, `matrix_screen`, `matrix_build`), `btl_param` + `btl_tech`.

**Not answered** (the two parts describe a structure or a variable differently and the script cannot choose):
`btl_act_1` + `btl_act_2`, `btl_char_fx_1` + `_2`, `eft_ribbon` + `eft_zap`, `eft_stage_2` + `eft_surface_out`,
`eft_speed_line_spawn` + `eft_aura`, `eft_body_fx` + `eft_disc`, `eft_glow` + `eft_trail`, `eft_link_1` + `_2`, `eft_link_2`
+ `eft_part10`, `hud_gauge_1` + `_2`, `hud_notice_1` + `_2`, `bracket_clips` + `bracket_logic`. `stg` + `stg_parts` have
a hand-written assembly chunk between them.

**Left under old names:**
- Headers. A header was renamed with its C file only where the two had the same name (68 of 179). The menu
  overlay's headers (`menu_b.h` .. `menu_za.h`) serve several screens each and keep their letters; some battle
  headers of merged-away or never-existing files keep theirs too (`eft_v_ext.h`, `hud_d.h`, `btl_capi_b.h`, ...).
  A renamed header that other files share now carries one file's name. A pass over the headers is its own job.
- `config/symbols/*.txt` (the name lists by address range) and the folders under `asm/` that are not generated
  from a C file's name.
- The other documents: they speak of the files by their old names. `docs/file_rename_map.txt` translates.
- The port repository's copy of the sources.
