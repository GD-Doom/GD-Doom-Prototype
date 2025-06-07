//----------------------------------------------------------------------------
//  EDGE Control Stuff
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

#include "dm_defs.h"
#include "dm_state.h"
#include "e_event.h"
#include "e_input.h"
#include "e_main.h"
#include "edge_profiling.h"
#include "epi.h"
#include "epi_str_util.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "platform/gd_platform.h"
#include "r_modes.h"

bool eat_mouse_motion = true;

void HandleFocusGain(void)
{
    // Hide cursor and grab input
    GrabCursor(true);

    // Ignore any pending mouse motion
    eat_mouse_motion = true;

    // Now active again
    app_state |= kApplicationActive;
}

void HandleFocusLost(void)
{
    GrabCursor(false);

    EdgeIdle();

    // No longer active
    app_state &= ~kApplicationActive;
}

/****** Input Event Generation ******/

int GetTime(void)
{
    uint32_t t = gd::Platform::GetTicks();

    // more complex than "t*35/1000" to give more accuracy
    return (t / 1000) * kTicRate + (t % 1000) * kTicRate / 1000;
}

int GetMilliseconds(void)
{
    return gd::Platform::GetTicks();
}

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
