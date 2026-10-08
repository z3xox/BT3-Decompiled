# Names for the headers

> **Applied on the branch `headers`, 2026-10-08** (local). Both outputs still equal the disc's from a fresh tree.
> The list as applied is `docs/header_rename_map.txt`; `scripts/rename_headers.py` renames from such a list and
> folds one header into another (`--fold EXTRA TARGET`). What was done is at the end; the draft follows as written.


A proposal only (2026-10-08). Nothing here is applied. The C files were renamed on the branch `rename`; a header was renamed with its C file only where both had the same name. This is the rest.

179 headers; 49 still have a placeholder name.

## How to read it

- "used by" is the C files that include the header directly. A header of ONE file takes that file's name; where the file already has a header of its name (it was merged from several), the others are `_part2`, `_part3`. Folding them into one header is the better end state but meets the same clash of declarations that stopped some file merges, so it is not proposed here.
- A header of SEVERAL files is named for what it holds. Where it holds two unrelated things the note says so: those are candidates for splitting, which is a separate step.
- A rename touches the header, every `#include` of it, and the mentions in comments. It changes nothing in the built game.

## Open choices

1. `_part2` for a second header of one file, or folding (see above).
2. `include/ui/` to go with `src/ui/` (today those files' headers are in `include/battle/` and `include/sys/`).
3. Splitting the headers that hold two things (`menu_l`, `menu_x`, `menu_u`).

## Headers with placeholder names

| now | lines | used by | proposed | note |
|---|---|---|---|---|
| `battle/bobj_b` | 401 | btl_obj_anim, btl_obj_chain | `battle/btl_obj_anim_part2` | second header of btl_obj_anim.c; btl_obj_chain.c uses it too |
| `battle/btl_act_b` | 106 | btl_act_1 | `battle/btl_act_1_part2` | second header of btl_act_1.c |
| `battle/btl_act_d` | 120 | btl_act_2 | `battle/btl_act_2_part2` | second header of btl_act_2.c |
| `battle/btl_act_e` | 123 | btl_act_2 | `battle/btl_act_2_part3` | third header of btl_act_2.c |
| `battle/btl_act_g` | 192 | btl_act_super | `battle/btl_act_super_part2` | second header of btl_act_super.c |
| `battle/btl_act_i` | 282 | btl_act_change | `battle/btl_act_change_part2` | second header of btl_act_change.c (decisions, skills) |
| `battle/btl_ai_seq_a` | 132 | btl_ai_seq | `battle/btl_ai_seq_part2` | second header of btl_ai_seq.c |
| `battle/btl_capi_b` | 387 | btl_char_api_2 | `battle/btl_char_api_2_part2` | second header of btl_char_api_2.c |
| `battle/eft_ac` | 419 | eft_ribbon | `battle/eft_ribbon_part2` | second header of eft_ribbon.c |
| `battle/eft_f` | 303 | eft_water | `battle/eft_water_part2` | second header of eft_water.c (the particle pools) |
| `battle/eft_l` | 398 | eft_obj_tech, eft_ring_shot, eft_absorb, eft_speed_line_spawn | `battle/eft_tech_modules` | shared by four technique effect files |
| `battle/eft_n` | 316 | eft_aura | `battle/eft_aura_part2` | second header of eft_aura.c (the bolts) |
| `battle/eft_t` | 388 | eft_shot_fx, eft_chain, eft_streak | `battle/eft_draw_modules` | shared by eft_shot_fx, eft_chain, eft_streak |
| `battle/eft_u` | 420 | eft_shot_fx, eft_particle | `battle/eft_shot_fx_particle` | shared by eft_shot_fx and eft_particle |
| `battle/eft_v` | 588 | (headers: eft_v_ext) | `(delete, or fold into eft_particle.h)` | no C file includes it |
| `battle/eft_v_ext` | 127 | eft_particle, eft_impact, eft_link_1 | `battle/eft_particle_ext` | what eft_particle, eft_impact and eft_link_1 take from elsewhere |
| `battle/hud_d` | 70 | hud_sprite, hud_notice_1 | `battle/hud_node` | the HUD node helpers; used by hud_sprite and hud_notice_1 |
| `battle/stg_d` | 101 | stg_rigid, stg_model_anim | `battle/stg_rigid_types` | rigid bodies; used by stg_rigid and stg_model_anim |
| `battle/view_b` | 296 | char_table, menu_util_2, menu_util_1, shen_scene | `ui/menu_support` | the main executable's menu support; four files in src/ui use it |
| `menu/menu_a` | 288 | main_menu, title, progress | `menu/overlay_common` | what every screen of the overlay takes from the main executable |
| `menu/menu_b` | 172 | mode_menu, mode_background, history_outro | `menu/mode_menu` | mode menu, its background, the history outro |
| `menu/menu_c` | 223 | history_select, history_guide, history_result, history_save | `menu/history` | the four Dragon History screens |
| `menu/menu_d` | 239 | char_select | `menu/char_select` | one user |
| `menu/menu_e` | 220 | team_select | `menu/team_select` | one user |
| `menu/menu_f` | 90 | team_select | `menu/team_select_part2` | second header of team_select.c |
| `menu/menu_g` | 176 | duel, duel_menu, item_panel | `menu/duel` | duel, duel menu, item panel |
| `menu/menu_h` | 205 | char_reference, reference_training_mode, training | `menu/char_reference` | character reference; training.c uses it too |
| `menu/menu_i` | 305 | boot_card, logo, entry_select, training | `menu/training` | mostly the training mode; boot_card, logo and entry_select use it too |
| `menu/menu_j` | 336 | entry_select, tour_menu | `menu/tour_entry` | entry select and tour menu |
| `menu/menu_k` | 299 | bracket, bracket_guide, tour_background, bracket_clips | `menu/bracket` | the bracket screen's files and the tour background |
| `menu/menu_l` | 470 | bracket_logic, solo_select | `menu/bracket_solo` | bracket logic and the solo select: two things in one header |
| `menu/menu_m` | 255 | ub_team_select | `menu/ub_team_select` | one user |
| `menu/menu_n` | 282 | ubz_select, ub_rank | `menu/ub_rank` | ranking ladder and course select |
| `menu/menu_o` | 415 | ub_result, ub, mission_select, disc_fusion | `menu/ub` | Ultimate Battle's handler, results, missions, disc fusion |
| `menu/menu_p` | 335 | mission_result, ub_menu, ub_score | `menu/ub_score` | score sheet, mission result, UB menu |
| `menu/menu_q` | 378 | sim_day | `menu/sim_day` | one user |
| `menu/menu_r` | 301 | sim_event, sim_result, sim_top | `menu/sim_top` | sim top, result, event runner |
| `menu/menu_s` | 281 | survival_select, survival_result | `menu/survival` | select and result |
| `menu/menu_t` | 195 | sim_event_1, sim_event_card | `menu/sim_event_card` | the card game and the first events |
| `menu/menu_u` | 302 | sim_event_33, sim_event_36, evo_z_1, sim_event_2, sim_event_35 ... | `menu/sim_event_evo_z` | eight users: the later Sim events and Evolution Z; a candidate for splitting |
| `menu/menu_v` | 297 | evo_z_3, item_help | `menu/evo_z_items` | Evolution Z part three and the item help |
| `menu/menu_w` | 199 | shop, evo_mode | `menu/shop` | shop and the Evolution Z mode handler |
| `menu/menu_x` | 226 | option_mode, option, evo_top | `menu/evo_top_option` | Evolution Z top menu and the options: two things |
| `menu/menu_y` | 97 | option | `menu/option` | one user |
| `menu/menu_z` | 460 | dc_list, dc, dc_menu, dc_password | `menu/dc` | the Data Center screens |
| `menu/menu_za` | 257 | password_window, password_check, replay_menu, dc_save | `menu/dc_password_replay` | password window and check, replay menu, save helper |
| `sys/gfxm_c` | 249 | flash | `sys/flash_player` | second header of flash.c: the player |
| `sys/gfxm_d` | 104 | flash | `sys/flash_clip_list` | third header of flash.c: the clip-list setters |
| `sys/lib_a` | 8 | nothing | `(delete)` | eight lines, nothing includes it |

## Renamed headers that several C files share

These took their C file's new name, and other files include them too. The name is not wrong, but it is one file's. Listed to look over; no change proposed.

| header | used by |
|---|---|
| `battle/btl_char_flag` | btl_clash, btl_opponent, btl_char_sound |
| `battle/btl_char_fx_1` | btl_char_fx_2 |
| `battle/btl_char_member` | btl_char_fx_1 |
| `battle/btl_demo_cam` | btl_cam, screen_fx |
| `battle/btl_facade` | btl_script, btl_script_cmd |
| `battle/btl_obj` | obj_draw |
| `battle/btl_pool` | eft_core, btl_scene, eft_shot, eft_stage_2 |
| `battle/btl_scene` | btl_char_mgr, btl_script_cmd, eft_stage_2, stg_ambient |
| `battle/btl_script` | btl_script_cmd |
| `battle/btl_stats` | btl_anim |
| `battle/col_primitives` | font |
| `battle/eft_disc` | eft_glow |
| `battle/eft_orb_tail` | eft_transform, eft_ribbon |
| `battle/eft_part10` | eft_layer3, eft_quad_1 |
| `battle/eft_quad_2` | eft_ground_dust |
| `battle/eft_rays` | eft_blast_obj, eft_body_fx |
| `battle/eft_surface_out` | eft_burst |
| `battle/eft_water` | eft_burst |
| `battle/eft_zap` | eft_mesh, eft_sprite |
| `battle/hud` | hud_team, hud_caption, hud_gauge_1 |
| `battle/hud_gauge_3` | hud_combo, hud_sprite |
| `battle/hud_notice_2` | hud_timer, btl_pause, hud_prompt |
| `battle/obj_gs_env` | model_tex, stg_reloc, obj_shadow |
| `battle/orbit_cam` | char_viewer |
| `battle/pause_menu` | btl_menu, btl_seq |
| `battle/stg` | stg_parts |
| `battle/stg_collision` | stg_nav, btl_ai_seq |
| `sys/bpe` | sprite |
| `sys/debug` | gfx, stg_ambient |
| `sys/fade` | movie, btl_script_cmd, screen_fx, char_viewer, shen_scene |
| `sys/gfx_screen` | tex_file |
| `sys/gsc` | btl_script, btl_script_cmd |
| `sys/job` | loading, battle, battle_load, shen_scene |
| `sys/list` | obj_draw |
| `sys/loading` | battle_work, battle_load |
| `sys/mem_align` | spu_heap |
| `sys/mem_read` | menu_util_2 |
| `sys/pad_watch` | btl_pause |
| `sys/password_old` | password_chara, dc_password |
| `sys/ramp` | screen_fx, shen_scene |
| `sys/rigid` | stg_rigid |
| `sys/sprite` | hud_sprite |
| `sys/tex_file` | flash |
| `sys/vu1_packet` | stg_model_anim, stg_model_draw |
| `ui/reward_window` | message_window, icon_window, char_viewer, menu_util_1 |
| `ui/shen_wish` | shen_confirm, shen_save |

## What was applied (2026-10-08)

The user's decisions: fold the extra headers where it works; split the headers that hold two things, but not the
bracket one ("it serves tournament mode in general"); delete nothing.

- **48 renames** as in the table, with these differences: `btl_ai_seq_a.h` -> `btl_ai_seq.h` and `eft_ac.h` ->
  `eft_ribbon.h` (the files had no header of their own name); `eft_v.h`, which nothing includes, is kept as
  `eft_particle_unused.h`; `menu_l.h` -> `tournament.h`, not split (it also holds the solo select's
  declarations); `lib_a.h` is kept as it is.
- **Folded into the file's main header** (each kept only with the build identical): `btl_act_1`, `btl_act_2` (two
  headers), `btl_act_super`, `btl_act_change`, `btl_char_api_2`, `eft_aura`, `team_select`, and `flash` (two
  headers): 11 headers gone into 8.
- **Not foldable**: `btl_obj_anim_part2.h` (`btl_obj_chain.c` includes it and its declarations clash with those of
  the main header there) and `eft_water_part2.h` (`gEftDust` is declared with two types). They keep `_part2`.
- **Split**:
  - `sim_event_evo_z.h` (eight users) -> `sim_events.h` (the Sim Dragon events) and `evo_z.h`. The Evolution Z
    files had been taking `Flash_ClipSetOffset` from the Sim half; without it one function compiled differently
    (five instructions), so `evo_z.h` declares it too.
  - `evo_top_option.h` -> `evo_top.h`; the options into the existing `option.h`, in front of what it had; and
    what both need (more of the main executable, the pad bits, the save helper) into `overlay_extra.h`.
- `include/ui/` exists now (`menu_support.h` joined `reward_window.h`); the other headers of `src/ui/` files are
  still in `include/battle/` and `include/sys/`.
- Left: the second table above (renamed headers that several files share), `config/symbols/*.txt`, and the
  mentions of old names in the other documents.
