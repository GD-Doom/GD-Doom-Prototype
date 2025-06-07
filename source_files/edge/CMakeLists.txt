##########################################
# edge
##########################################

set (EDGE_SOURCE_FILES
  i_movie.cc
  i_ctrl.cc
  i_video.cc
  i_sound.cc
  am_map.cc
  bot_nav.cc
  bot_think.cc
  con_con.cc
  con_main.cc
  con_var.cc
  e_input.cc
  e_main.cc
  e_player.cc
  f_finale.cc
  f_interm.cc
  g_game.cc
  hu_draw.cc
  hu_font.cc
  hu_stuff.cc
  hu_style.cc
  i_system.cc
  im_filter.cc
  im_data.cc
  im_funcs.cc
  l_deh.cc
  m_argv.cc
  m_bbox.cc
  m_cheat.cc
  m_math.cc
  m_menu.cc
  m_misc.cc
  m_option.cc
  m_netgame.cc
  m_random.cc
  n_network.cc
  p_action.cc
  p_blockmap.cc
  p_enemy.cc
  p_inter.cc
  p_lights.cc
  p_map.cc
  p_maputl.cc
  p_mobj.cc
  p_plane.cc
  p_setup.cc
  p_sight.cc
  p_spec.cc
  p_switch.cc
  p_tick.cc
  p_user.cc
  p_forces.cc
  p_telept.cc
  p_umapinfo.cc
  p_weapon.cc
  rad_act.cc
  rad_pars.cc
  rad_trig.cc
  r_bsp.cc
  r_draw.cc
  r_shader.cc
  r_render.cc
  r_effects.cc
  r_backend.cc
  r_occlude.cc
  r_things.cc
  r_units.cc
  r_wipe.cc
  r_misc.cc
  r_sky.cc  
  r_colormap.cc
  r_modes.cc
  r_image.cc
  r_doomtex.cc
  r_texgl.cc
  s_blit.cc
  s_cache.cc
  s_doom.cc
  s_flac.cc
  s_m4p.cc
  s_midi.cc
  s_mp3.cc
  s_music.cc
  s_ogg.cc
  s_sound.cc
  s_wav.cc
  sv_chunk.cc
  sv_glob.cc
  sv_level.cc
  sv_load.cc
  sv_main.cc
  sv_misc.cc
  sv_mobj.cc
  sv_play.cc
  sv_save.cc
  snd_data.cc
  snd_gather.cc
  snd_types.cc
  w_files.cc
  w_flat.cc
  w_epk.cc
  w_sprite.cc
  w_texture.cc
  w_wad.cc
  platform/gd_platform.cc
  script/lua_debugger.cc
  script/compat/lua_vm.cc
  script/compat/lua_compat.cc
  script/compat/lua_core.cc
  script/compat/lua_game.cc
  script/compat/lua_player.cc
  script/compat/lua_hud.cc)
  
if (EDGE_SOKOL_GL)

  set (EDGE_SOKOL_SOURCE_FILES
    render/sokol/sokol_backend.cc
    render/sokol/sokol_gl.cc  
    render/sokol/sokol_images.cc
    render/sokol/sokol_pipeline.cc
    render/sokol/sokol_sky.cc
    render/sokol/sokol_state.cc
    render/sokol/sokol_units.cc
  )

  list (APPEND EDGE_SOURCE_FILES ${EDGE_SOKOL_SOURCE_FILES})

else()

  set (EDGE_NULL_SOURCE_FILES
    render/null/null_backend.cc
    render/null/null_state.cc
    render/null/null_units.cc
  )

  list (APPEND EDGE_SOURCE_FILES ${EDGE_NULL_SOURCE_FILES})

endif()

# Platform sources
if (GD_PLATFORM_SDL)

  set (GD_PLATFORM_SDL_SOURCE_FILES
    platform/sdl/gd_platform_sdl.cc
  )

  list (APPEND EDGE_SOURCE_FILES ${GD_PLATFORM_SDL_SOURCE_FILES})

else()

  set (GD_PLATFORM_NULL_SOURCE_FILES
    platform/null/gd_platform_null.cc
  )

  list (APPEND EDGE_SOURCE_FILES ${GD_PLATFORM_NULL_SOURCE_FILES})

endif()

list (APPEND EDGE_SOURCE_FILES i_main.cc)

if (WIN32)
  list (APPEND EDGE_SOURCE_FILES w32_res.rc)
endif()

add_executable(
  edge-classic
  ${EDGE_SOURCE_FILES}
)

if(WIN32)
  target_compile_definitions(edge-classic PRIVATE WIN32)
else()
  target_compile_definitions(edge-classic PRIVATE UNIX)
endif()

if(WIN32)
  target_include_directories(edge-classic PRIVATE ${EDGE_LIBRARY_DIR}/sdl2/include)
endif()

target_include_directories(edge-classic PRIVATE ./)

set (EDGE_LINK_LIBRARIES
  ajbsp
  ddf
  dehacked
  epi
  edge_tracy  
  almostequals
  fluidlite
  HandmadeMath
  lua
  m4p
  miniaudio
  minivorbis
  miniz
  pl_mpeg
  prns
  stb
)

if (GD_PLATFORM_SDL)
  list (APPEND EDGE_LINK_LIBRARIES thread ${SDL2_LIBRARIES})
elseif(WIN32)
    set_target_properties(edge-classic PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

if (NOT EMSCRIPTEN)
  list (APPEND EDGE_LINK_LIBRARIES thread)
endif()

if (EDGE_PROFILING)
  list (APPEND EDGE_LINK_LIBRARIES TracyClient)
endif()

if (EDGE_SOKOL_GL)
  list(APPEND EDGE_LINK_LIBRARIES sokol)
endif()

target_link_libraries(edge-classic PRIVATE ${EDGE_LINK_LIBRARIES})

target_compile_options(edge-classic PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:${EDGE_WARNING_LEVEL}>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${EDGE_WARNING_LEVEL}>
)

set(COPY_FILES "")


set (DEST_DIR "${CMAKE_SOURCE_DIR}")
list(APPEND COPY_FILES "$<TARGET_FILE:edge-classic>")

if (GD_PLATFORM_SDL)

  # Copy appropriate SDL2.dll to local install directory when built with MSVC/Clang
  if (WIN32)
    if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
      list(APPEND COPY_FILES "${CMAKE_SOURCE_DIR}/libraries/sdl2/x64/SDL2.dll")
    else ()
      list(APPEND COPY_FILES "${CMAKE_SOURCE_DIR}/libraries/sdl2/x86/SDL2.dll")
    endif ()
  endif()
endif()

if (COPY_FILES)
  add_custom_command( TARGET edge-classic POST_BUILD COMMAND_EXPAND_LISTS COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COPY_FILES} ${DEST_DIR})
endif()


