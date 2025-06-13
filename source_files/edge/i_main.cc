//----------------------------------------------------------------------------
//  EDGE Main
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
#include "e_main.h"
#include "epi_filesystem.h"
#ifdef GD_PLATFORM_SDL
#include "epi_sdl.h"
#endif
#include "epi_str_util.h"
#include "i_system.h"
#include "m_argv.h"
#include "platform/gd_platform.h"
#include "version.h"

std::string executable_path = ".";

extern "C"
{
    int main(int argc, char *argv[])
    {
        gd::Platform_Init();

        executable_path = gd::Platform::GetBasePath();

        EdgeMain(argc, (const char **)argv);
        EdgeShutdown();
        SystemShutdown();
        return EXIT_SUCCESS;
    }

} // extern "C"

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
