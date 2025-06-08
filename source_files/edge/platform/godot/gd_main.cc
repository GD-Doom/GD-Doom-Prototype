#include "dm_defs.h"
#include "e_main.h"
#include "epi_filesystem.h"
#include "epi_str_util.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "r_modes.h"
#include "version.h"

#include "platform/gd_platform.h"

#ifdef WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif


extern std::string executable_path;

void GDD_Tick()
{
    if (app_state & kApplicationActive)
        EdgeTicker();
}

void GDD_Init(int argc, char *argv[])
{

#ifdef WIN32
    executable_path = "C:\\Dev\\GD-Doom-Prototype";//SDL_GetBasePath();
    SetCurrentDirectory("C:\\Dev\\GD-Doom-Prototype");
#else
    //argc = 2;
    //const char* argvv[] = {"-iwad", "/home/jenge/Dev/GD-Doom-Prototype/DOOM2.WAD"};
    //argv = (char**) argvv;
    executable_path = "/home/jenge/Dev/GD-Doom-Prototype";//SDL_GetBasePath();
    chdir(executable_path.c_str());
#endif

    gd::Platform_Init();

    EdgeMain(argc, (const char **)argv);

    // EdgeShutdown();
    // SystemShutdown();
}


