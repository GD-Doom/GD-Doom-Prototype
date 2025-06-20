#include "dm_defs.h"
#include "e_main.h"
#include "epi_filesystem.h"
#include "epi_str_util.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "platform/gd_platform.h"
#include "r_modes.h"
#include "version.h"

#ifdef WIN32
#include "windows.h"
#else
#include <unistd.h>
#endif

#include <godot_cpp/classes/project_settings.hpp>

#include <codecvt> // For std::codecvt_utf8
#include <locale>  // For std::wstring_convert

using namespace godot;

extern std::string executable_path;

void GDD_Tick()
{
    if (app_state & kApplicationActive)
        EdgeTicker();
}

static std::string u32_to_string(const std::u32string &u32str)
{
    // Create a wstring_convert object for UTF-32 to UTF-8 conversion
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.to_bytes(u32str);
}

void GDD_Init(int argc, char *argv[])
{

    // THIS IS TEMPORARY BOOTSTRAPPING CODE

    ProjectSettings *project_settings = ProjectSettings::get_singleton();

    String root_folder = project_settings->globalize_path("res://") + "/..";
    root_folder        = root_folder.simplify_path();

    executable_path = u32_to_string(std::u32string(root_folder.ptr()));

#ifdef WIN32
    SetCurrentDirectory(executable_path.c_str());
#else
    int _ = chdir(executable_path.c_str());
    EPI_UNUSED(_);
#endif

    gd::Platform_Init();

    EdgeMain(argc, (const char **)argv);

    // EdgeShutdown();
    // SystemShutdown();
}
