# Draft: readable names for the C files

A proposal only (2026-10-08). Nothing here is applied. Written at the user's request to read and change.

210 of the 273 C files would get a new name; 63 keep theirs.

## How to read it

- The names follow the functions inside: every function already has a real name with a module prefix (`HudGauge_`, `EftRibbon_`, `CharSel_`), and a file is named for the module it holds.
- **clear**: one module, or one that is most of the file. **mixed**: several modules share the file and it is named for the largest; a better name may need the file to be split, which is a separate question. **slice**: one part of a larger original source file whose other parts are separate files here.
- "functions" counts the definitions and the ones still in assembly. "modules" are the three most common prefixes with their counts.
- A rename changes nothing in the built game. Each one also touches the file's header in include/, its line in the two build lists (config/*.yaml), its symbol list in config/symbols/, the `#include` lines elsewhere and the docs. The port repository has its own copy of these files and must follow in the same step.

## Open choices

1. **A folder `src/ui/`** for the menu support code that lives in the main executable (windows, text box, character viewer, the dragon screen). Today it is spread over `battle/view_*` and `sys/late_*`. Without it those files would stay in `battle/` and `sys/` under their new names.
2. **Short game words in menu names**: `ub` (Ultimate Battle), `sim` (Sim Dragon), `dc` (Data Center), `hist` (Dragon History), `tour` (World Tour), `evo` (Evolution Z), `shen` (the dragon). They match the function prefixes. Spelled out they are longer (`ultimate_battle_rank.c`).
3. **Parts of one module** are `name`, `name_b` (`eft_quad`, `eft_quad_b`). The alternative is `_1`, `_2`.
4. **The effect files** (`eft_*`, 57 of them) have no better grouping than their module; several are mixed.

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
| `sys/gfxm_e_c` | `battle/mdl_tex` | 1 | MdlTex 1 | clear |  |
| `sys/gfxm_e_d` | `battle/stg_reloc` | 2 | StgOctree 1, BtlStage 1 | mixed |  |
| `sys/late_a` | `ui/shen` | 33 | Shen 25, ShenList 6 | clear | main-executable menu screens: a new folder ui/ for them and for view_* |
| `sys/late_a_b` | `ui/shen_confirm` | 14 | ShenCfm 14 | clear |  |
| `sys/late_a_c` | `ui/shen_save` | 8 | ShenSave 8 | clear |  |
| `sys/mcflow_a` | `sys/mc_flow` | 22 | McFlow 22 | clear |  |
| `sys/misc_a` | `sys/pass_old` | 21 | OldPass 21 | clear |  |
| `sys/misc_a_b` | `sys/pass_chara` | 18 | ChrPass 18 | clear |  |
| `sys/randf` | `sys/rand_util` | 5 | Rand 5 | clear |  |
| `sys/vu0_a_c` | `sys/ivec` | 18 | IVec4 9, IVec3 8, IVec 1 | mixed |  |
| `sys/vu0_a_c_b` | `sys/mtx_screen` | 1 | Mtx 1 | clear |  |
| `sys/vu0_a_c_c` | `sys/mtx_build` | 15 | Mtx 8, Vu0Cur 7 | mixed |  |
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
| `battle/btl_char_coll_b` | `battle/btl_hit_react` | 25 | BtlColl 25 | clear |  |
| `battle/btl_char_ctl_b` | `battle/btl_change` | 14 | BtlChange 13, BtlChars 1 | clear |  |
| `battle/btl_char_ctl_c` | `battle/btl_head_track` | 6 | BtlChar 6 | clear |  |
| `battle/btl_char_ctl_d` | `battle/btl_char_pose` | 28 | BtlChar 26, BtlChars 2 | clear |  |
| `battle/btl_char_flag_clash` | `battle/btl_clash` | 15 | BtlClash 15 | clear |  |
| `battle/btl_char_flag_opp` | `battle/btl_opp` | 37 | BtlOpp 37 | clear |  |
| `battle/btl_char_flag_snd` | `battle/btl_char_snd` | 23 | BtlCharSnd 23 | clear |  |
| `battle/btl_char_fx_c` | `battle/btl_char_fx_b` | 39 | BtlFx 18, BtlPartner 15, BtlFxObj 4 | mixed |  |
| `battle/btl_char_get` | `battle/btl_char_util` | 39 | BtlChar 26, BtlUtil 13 | clear |  |
| `battle/btl_char_status` | `battle/btl_stat` | 37 | BtlStat 37 | clear |  |
| `battle/btl_char_status_anim` | `battle/btl_anim` | 42 | BtlAnim 42 | clear |  |
| `battle/btl_tech_a` | `battle/btl_param` | 127 | BtlParam 51, BtlAtk 46, BtlCtrl 30 | mixed |  |
| `battle/btl_tech_b` | `battle/btl_tech` | 182 | BtlKiBlast 66, BtlSuper 51, BtlSkill 34 | mixed |  |
| `battle/col_a` | `battle/col_box` | 27 | ColBox 11, ColMesh 7, ColObb 4 | mixed |  |
| `battle/col_b` | `battle/col_prim` | 42 | ColSeg 10, ColSphere 10, ColBounds 5 | mixed |  |
| `battle/col_c` | `sys/font` | 101 | Font 101 | clear | not collision: the text printer; moves to sys/ |
| `battle/col_c_b` | `sys/font_icon` | 28 | FontIcon 20, FontTag 8 | clear |  |
| `battle/eft_a` | `battle/eft_core` | 91 | EftHit 48, EftHitArena 10, EftCam 10 | mixed |  |
| `battle/eft_aa` | `battle/eft_char_parts` | 84 | EftAnimPart 17, EftCharSlot 15, EftBlade 13 | mixed | grab-bag of small per-character parts |
| `battle/eft_ab` | `battle/eft_orb_tail` | 35 | EftOrbTail 32, EftOrbTailMgr 3 | clear |  |
| `battle/eft_ab_b` | `battle/eft_transform` | 17 | EftTransform 14, EftTransformMgr 3 | clear |  |
| `battle/eft_ab_c` | `battle/eft_ribbon` | 60 | EftRibbon 35, EftZap 12, EftZapMgr 5 | clear |  |
| `battle/eft_ad` | `battle/eft_zap` | 29 | EftZap 19, EftShock 7, EftShockMgr 3 | clear |  |
| `battle/eft_ad_b` | `battle/eft_mesh` | 36 | EftMesh 26, EftObj 10 | clear |  |
| `battle/eft_ad_c` | `battle/eft_spr` | 2 | EftSpr 2 | clear |  |
| `battle/eft_ae` | `battle/eft_spr_anim` | 93 | EftSprAnim 31, BtlTask 17, BtlTaskList 14 | mixed | also holds the battle task tree (BtlTask, 31 functions): could be split in two names only if the file is split |
| `battle/eft_b` | `battle/eft_stage_a` | 49 | EftBubble 23, EftGeyser 13, EftStageScroll 8 | mixed |  |
| `battle/eft_c` | `battle/eft_stage_b` | 77 | EftSurf 25, EftWeather 23, EftStage 15 | mixed |  |
| `battle/eft_d` | `battle/eft_surf_out` | 12 | EftSurf 12 | clear |  |
| `battle/eft_d_b` | `battle/eft_burst` | 39 | EftBurst 33, EftBurstLayer 3, Eft 2 | clear |  |
| `battle/eft_det_a` | `battle/eft_detect` | 40 | EftDet 18, EftVolleyAim 8, StgGround 5 | mixed |  |
| `battle/eft_det_b` | `battle/stg_col` | 33 | StgCol 25, StgShadow 8 | clear |  |
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
| `battle/eft_l_d` | `battle/eft_spd_line_spawn` | 2 | EftSpdLine 2 | clear |  |
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
| `battle/eft_u_b` | `battle/eft_ptcl` | 40 | EftPtcl 36, EftPtclMgr 4 | clear |  |
| `battle/eft_v_b` | `battle/eft_impact` | 16 | EftImpact 12, EftImpactMgr 4 | clear |  |
| `battle/eft_v_c` | `battle/eft_link` | 15 | EftLink 11, EftLinkMgr 4 | clear |  |
| `battle/eft_w` | `battle/eft_link_b` | 44 | EftLink 31, EftPart10 9, EftPart10Mgr 4 | clear |  |
| `battle/eft_x` | `battle/eft_part10` | 34 | EftPart10 34 | clear |  |
| `battle/eft_x_b` | `battle/eft_layer3` | 4 | EftLayer3 4 | clear |  |
| `battle/eft_x_c` | `battle/eft_quad` | 12 | EftQuad 8, EftQuadMgr 4 | clear |  |
| `battle/eft_y` | `battle/eft_quad_b` | 35 | EftQuad 35 | clear |  |
| `battle/eft_z` | `battle/eft_bill` | 65 | EftBill 20, EftGndDust 11, EftLine 6 | mixed |  |
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
| `battle/stg_b` | `battle/stg_amb` | 89 | StgAmb 33, BtlStage 18, ScrWarp 17 | mixed |  |
| `battle/stg_c` | `battle/scr_fx` | 69 | StgHaze 22, StgBlur 21, StgTint 12 | mixed |  |
| `battle/stg_d` | `battle/stg_rigid` | 20 | StgRigid 12, StgRigidList 7, StgAabb 1 | clear |  |
| `battle/stg_d_b` | `battle/stg_model_anim` | 5 | StgModel 5 | clear |  |
| `battle/stgm_a` | `battle/stg_model_draw` | 13 | StgModel 13 | clear |  |
| `battle/stgm_a_b` | `sys/mc_card` | 20 | McCard 19, McCardFile 1 | clear | not stage code: memory card operations; moves to sys/ |
| `battle/view_a` | `ui/get_win` | 8 | GetWin 8 | clear |  |
| `battle/view_a_b` | `ui/msg_win` | 8 | MsgWin 8 | clear |  |
| `battle/view_a_c` | `ui/icon_win` | 6 | IconWin 6 | clear |  |
| `battle/view_a_d` | `ui/chr_view` | 19 | ChrView 19 | clear |  |
| `battle/view_a_e` | `ui/menu_util` | 40 | ChrGrid 9, Num 7, StgGrid 7 | mixed | Progress_Init, the character and stage grids, number drawing, the head of the text box |
| `battle/view_b` | `ui/text_box` | 10 | TextBox 9 | clear |  |
| `battle/view_b_b` | `ui/chr_tbl` | 11 | ChrTbl 5, ItemSet 4, ItemTbl 2 | mixed |  |
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
| `menu/menu_b_b` | `menu/mode_bg` | 3 | ModeBg 3 | clear |  |
| `menu/menu_b_c` | `menu/hist_outro` | 9 | HistOutro 8, Hist 1 | clear |  |
| `menu/menu_b_d` | `menu/hist_sel` | 11 | HistSel 11 | clear |  |
| `menu/menu_c_b` | `menu/hist_guide` | 2 | HistGuide 2 | clear |  |
| `menu/menu_c_c` | `menu/hist_result` | 11 | HistResult 11 | clear |  |
| `menu/menu_c_d` | `menu/hist_save` | 7 | HistSave 7 | clear |  |
| `menu/menu_c_e` | `menu/char_sel` | 16 | CharSel 16 | clear |  |
| `menu/menu_e_b` | `menu/team_sel` | 21 | TeamSel 21 | clear |  |
| `menu/menu_g` | `menu/item_panel` | 9 | ItemPanel 9 | clear |  |
| `menu/menu_g_b` | `menu/duel` | 2 | Duel 2 | clear |  |
| `menu/menu_g_c` | `menu/duel_menu` | 12 | DuelMenu 12 | clear |  |
| `menu/menu_h_b` | `menu/char_ref` | 20 | CharRef 20 | clear |  |
| `menu/menu_h_c` | `menu/ref_train_mode` | 2 | CharRefMode 1, TrainMode 1 | mixed |  |
| `menu/menu_h_d` | `menu/train` | 47 | Train 46 | clear |  |
| `menu/menu_i_b` | `menu/boot_card` | 4 | BootCard 4 | clear |  |
| `menu/menu_i_c` | `menu/logo` | 7 | Logo 4, FirstRun 3 | mixed |  |
| `menu/menu_i_d` | `menu/entry_sel` | 14 | EntrySel 13, Tour 1 | clear |  |
| `menu/menu_j_b` | `menu/tour_menu` | 9 | TourMenu 9 | clear |  |
| `menu/menu_k_b` | `menu/bracket` | 7 | Bracket 7 | clear |  |
| `menu/menu_k_c` | `menu/bracket_guide` | 2 | Bracket 2 | clear |  |
| `menu/menu_k_d` | `menu/tour_bg` | 3 | TourBg 3 | clear |  |
| `menu/menu_k_e` | `menu/bracket_clips` | 1 | Bracket 1 | clear |  |
| `menu/menu_k_f` | `menu/bracket_logic` | 21 | Bracket 21 | clear |  |
| `menu/menu_l_b` | `menu/solo_sel` | 12 | SoloSel 12 | clear |  |
| `menu/menu_m_b` | `menu/ub_team_sel` | 17 | UbTeamSel 17 | clear |  |
| `menu/menu_n_b` | `menu/ubz_sel` | 9 | UbzSel 9 | clear |  |
| `menu/menu_n_c` | `menu/ub_rank` | 14 | UbRank 14 | clear |  |
| `menu/menu_n_d` | `menu/ub_result` | 8 | UbResult 8 | clear |  |
| `menu/menu_o_b` | `menu/disc_fusion` | 11 | DiscFusion 11 | clear |  |
| `menu/menu_o_c` | `menu/ub` | 4 | Ub 4 | clear |  |
| `menu/menu_o_d` | `menu/mis_sel` | 10 | MisSel 10 | clear |  |
| `menu/menu_p_b` | `menu/mis_result` | 10 | MisResult 10 | clear |  |
| `menu/menu_p_c` | `menu/ub_menu` | 10 | UbMenu 10 | clear |  |
| `menu/menu_p_d` | `menu/ub_score` | 13 | UbScore 13 | clear |  |
| `menu/menu_q_b` | `menu/sim_day` | 30 | SimDay 30 | clear |  |
| `menu/menu_r_b` | `menu/sim_event` | 1 | SimEvent 1 | clear |  |
| `menu/menu_r_c` | `menu/sim_result` | 8 | SimResult 8 | clear |  |
| `menu/menu_r_d` | `menu/sim_top` | 9 | SimTop 9 | clear |  |
| `menu/menu_s_b` | `menu/surv_sel` | 9 | SurvSel 9 | clear |  |
| `menu/menu_s_c` | `menu/surv_result` | 7 | SurvResult 7 | clear |  |
| `menu/menu_s_d` | `menu/sim_ev_a` | 31 | SimEv00 1, SimEv01 1, SimEv02 1 | mixed |  |
| `menu/menu_t_b` | `menu/sim_ev_card` | 7 | SimEv28 4 | clear |  |
| `menu/menu_u` | `menu/sim_ev_b` | 1 |  | clear | check before naming: the stem covers eight source files and this one holds one function |
| `menu/menu_u_b` | `menu/sim_ev_rock` | 2 | SimRock 1 | clear |  |
| `menu/menu_u_c` | `menu/sim_ev_33` | 1 |  | clear |  |
| `menu/menu_u_d` | `menu/sim_ev_shell` | 4 | SimPopo 3 | clear |  |
| `menu/menu_u_e` | `menu/sim_ev_35` | 1 |  | clear |  |
| `menu/menu_u_f` | `menu/sim_ev_36` | 1 |  | clear |  |
| `menu/menu_u_g` | `menu/evo_z` | 5 | EvoZ 5 | clear |  |
| `menu/menu_u_h` | `menu/evo_z_clips` | 4 | EvoZ 4 | clear |  |
| `menu/menu_v_b` | `menu/evo_z_b` | 13 | EvoZ 13 | clear |  |
| `menu/menu_v_c` | `menu/item_help` | 5 | ItemHelp 5 | clear |  |
| `menu/menu_v_d` | `menu/shop` | 16 | Shop 16 | clear |  |
| `menu/menu_w_b` | `menu/evo_mode` | 1 | EvoMode 1 | clear |  |
| `menu/menu_w_c` | `menu/evo_top` | 20 | EvoTop 20 | clear |  |
| `menu/menu_x_b` | `menu/opt_mode` | 1 | OptMode 1 | clear |  |
| `menu/menu_x_c` | `menu/option` | 14 | Option 14 | clear |  |
| `menu/menu_y_b` | `menu/dc_list` | 67 | DcList 42, DcView 17, DcChars 5 | clear |  |
| `menu/menu_z_b` | `menu/dc` | 1 | Dc 1 | clear |  |
| `menu/menu_z_c` | `menu/dc_menu` | 22 | DcMenu 22 | clear |  |
| `menu/menu_z_d` | `menu/dc_pass` | 68 | DcPass 60, DcPassText 5, DcPassList 1 | clear |  |
| `menu/menu_za_b` | `menu/pass_win` | 10 | PassWin 10 | clear |  |
| `menu/menu_za_c` | `menu/pass_chk` | 6 | PassChk 6 | clear |  |
| `menu/menu_za_d` | `menu/replay_menu` | 36 | ReplayMenu 36 | clear |  |
| `menu/menu_za_e` | `menu/dc_save` | 8 | DcSave 8 | clear |  |

## Kept as they are

`sys/adx`, `sys/bpe`, `sys/color_fade`, `sys/common`, `sys/debug`, `sys/dialog`, `sys/dma`, `sys/fade`, `sys/file`, `sys/game_pad`, `sys/gfx`, `sys/gfx_ot`, `sys/gsc`, `sys/heap`, `sys/heap_info`, `sys/iop_heap`, `sys/job`, `sys/list`, `sys/loading`, `sys/math3d`, `sys/mathf`, `sys/mem_read`, `sys/movie`, `sys/pad`, `sys/pad_watch`, `sys/queue`, `sys/ramp`, `sys/rand`, `sys/rigid`, `sys/save`, `sys/snd`, `sys/sprite`, `sys/spu_heap`, `sys/timer`, `sys/vu1_packet`, `battle/battle`, `battle/battle_load`, `battle/battle_work`, `battle/btl_ai_mgr`, `battle/btl_ai_seq`, `battle/btl_cam`, `battle/btl_char_action`, `battle/btl_char_cam`, `battle/btl_char_cam_cut`, `battle/btl_char_cam_modes`, `battle/btl_char_flag`, `battle/btl_char_fx`, `battle/btl_char_hit`, `battle/btl_char_member`, `battle/btl_char_mgr`, `battle/btl_char_move`, `battle/btl_demo_cam`, `battle/btl_facade`, `battle/btl_input`, `battle/btl_obj`, `battle/btl_pool`, `battle/btl_replay`, `battle/btl_scene`, `battle/btl_script`, `battle/btl_script_cmd`, `battle/btl_seq`, `battle/orbit_cam`, `cri/adxt`
