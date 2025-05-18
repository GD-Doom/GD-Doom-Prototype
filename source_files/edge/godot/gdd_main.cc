#include "dm_defs.h"
#include "e_main.h"
#include "epi_filesystem.h"
#include "epi_sdl.h" // needed for proper SDL main linkage
#include "epi_str_util.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "r_modes.h"
#include "version.h"

#ifdef WIN32
#include "windows.h"
#endif

extern std::string executable_path;

void GDD_Tick()
{
    if (app_state & kApplicationActive)
        EdgeTicker();
}

void GDD_Init(int argc, char *argv[])
{
#ifdef EDGE_MIMALLOC
    if (SDL_SetMemoryFunctions(mi_malloc, mi_calloc, mi_realloc, mi_free) != 0)
        FatalError("Couldn't init SDL memory functions!!\n%s\n", SDL_GetError());
#endif

#ifdef WIN32
    SetCurrentDirectory("C:\\Dev\\GD-Doom-Prototype");
#endif

    if (SDL_Init(0) < 0)
        FatalError("Couldn't init SDL!!\n%s\n", SDL_GetError());

    executable_path = "C:\\Dev\\GD-Doom-Prototype";//SDL_GetBasePath();

    EdgeMain(argc, (const char **)argv);
}
