//----------------------------------------------------------------------------
//  EDGE SDL Video Code
//----------------------------------------------------------------------------
//
//  Copyright (c) 1999-2024 The EDGE Team.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------

#include "i_video.h"

#include "con_main.h"
#include "ddf_main.h"
#include "dm_state.h"
#include "edge_profiling.h"
#include "epi_str_compare.h"
#include "epi_str_util.h"
#include "i_defs_gl.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "n_network.h"
#include "r_backend.h"
#include "r_modes.h"
#include "r_state.h"
#include "version.h"
#include "platform/gd_platform.h"

int graphics_shutdown = 0;

// I think grab_mouse should be an internal bool instead of a cvar...why would a
// user need to adjust this on the fly? - Dasho
EDGE_DEFINE_CONSOLE_VARIABLE(grab_mouse, "1", kConsoleVariableFlagArchive)
EDGE_DEFINE_CONSOLE_VARIABLE(vsync, "1", kConsoleVariableFlagArchive)
EDGE_DEFINE_CONSOLE_VARIABLE_CLAMPED(gamma_correction, "0", kConsoleVariableFlagArchive, -1.0, 1.0)

// this is the Monitor Size setting, really an aspect ratio.
// it defaults to 16:9, as that is the most common monitor size nowadays.
EDGE_DEFINE_CONSOLE_VARIABLE(monitor_aspect_ratio, "1.77777", kConsoleVariableFlagArchive)

// these are zero until StartupGraphics is called.
// after that they never change (we assume the desktop won't become other
// resolutions while EC is running).
EDGE_DEFINE_CONSOLE_VARIABLE(desktop_resolution_width, "0", kConsoleVariableFlagReadOnly)
EDGE_DEFINE_CONSOLE_VARIABLE(desktop_resolution_height, "0", kConsoleVariableFlagReadOnly)

EDGE_DEFINE_CONSOLE_VARIABLE(pixel_aspect_ratio, "1.0", kConsoleVariableFlagReadOnly);

// when > 0, this will force the pixel_aspect to a particular value, for
// cases where a normal logic fails.  however, it will apply to *all* modes,
// including windowed mode.
EDGE_DEFINE_CONSOLE_VARIABLE(forced_pixel_aspect_ratio, "0", kConsoleVariableFlagArchive)
EDGE_DEFINE_CONSOLE_VARIABLE(framerate_limit, "500", kConsoleVariableFlagArchive)

bool grab_state;

extern ConsoleVariable renderer_far_clip;
extern ConsoleVariable draw_culling;
extern ConsoleVariable draw_culling_distance;

extern bool need_mouse_recapture;

void GrabCursor(bool enable)
{
    if (!gd::Platform::GetProgramWindow() || graphics_shutdown)
        return;

    grab_state = enable;

    if (grab_state)
        need_mouse_recapture = false;
    else
        need_mouse_recapture = true;

    if (grab_state && grab_mouse.d_)
    {
        gd::Platform::SetRelativeMouseMode(true);
    }
    else
    {
        gd::Platform::SetRelativeMouseMode(false);
    }
}

void DeterminePixelAspect()
{
    // the pixel aspect is the shape of pixels on the monitor for the current
    // video mode.  on modern LCDs (etc) it is usuall 1.0 (1:1).  knowing this
    // is critical to get things drawn correctly.  for example, Doom assets
    // assumed a 320x200 resolution on a 4:3 monitor, a pixel aspect of 5:6 or
    // 0.833333, and we must adjust image drawing to get "correct" results.

    // allow user to override
    if (forced_pixel_aspect_ratio.f_ > 0.1)
    {
        pixel_aspect_ratio = forced_pixel_aspect_ratio.f_;
        return;
    }

    // if not a fullscreen mode, check for a modern LCD (etc) monitor -- they
    // will have square pixels (1:1 aspect).
    bool is_crt = (desktop_resolution_width.d_ < desktop_resolution_height.d_ * 7 / 5);

    bool is_fullscreen = (current_window_mode == kWindowModeBorderless);
    if (is_fullscreen && current_screen_width == desktop_resolution_width.d_ &&
        current_screen_height == desktop_resolution_height.d_ && graphics_shutdown)
        is_fullscreen = false;

    if (!is_fullscreen && !is_crt)
    {
        pixel_aspect_ratio = 1.0f;
        return;
    }

    // in fullscreen modes, or a CRT monitor, compute the pixel aspect from
    // the current resolution and Monitor Size setting.  this assumes that the
    // video mode is filling the whole monitor (i.e. the monitor is not doing
    // any letter-boxing or pillar-boxing).  DPI setting does not matter here.

    pixel_aspect_ratio = monitor_aspect_ratio.f_ * (float)current_screen_height / (float)current_screen_width;
}

void StartupGraphics(void)
{
    gd::Platform::StartupGraphics();
}

bool SetScreenSize(DisplayMode *mode)
{
    return gd::Platform::SetScreenSize(mode);
}

void StartFrame(void)
{
    gd::Platform::StartFrame();
}   

void FinishFrame(void)
{
    gd::Platform::FinishFrame();
}

void ShutdownGraphics(void)
{
    gd::Platform::ShutdownGraphics();
}

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
