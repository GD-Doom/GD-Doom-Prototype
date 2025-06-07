
#include <dm_defs.h>
#include <epi_str_compare.h>
#include <epi_str_util.h>

#include "../../con_main.h"
#include "../../dm_state.h"
#include "../../e_input.h"
#include "../../e_main.h"
#include "../../i_system.h"
#include "../../m_argv.h"
#include "../../n_network.h"
#include "../../r_backend.h"
#include "../../r_modes.h"
#include "../../r_state.h"
#include "../../version.h"
#include "../gd_platform.h"

bool alt_is_down          = false;
bool need_mouse_recapture = false;
int  joystick_device      = 0;

void ShowGamepads(void)
{
}

int JoystickGetAxis(int n)
{
    return 0;
}

namespace gd
{
class NullPlatform : public Platform
{
    uint32_t ticks_ = 0;

  protected:
    uint32_t GetTicksInternal() override
    {
        return ticks_++;
    }

    std::string GetBasePathInternal(void) override
    {
        return std::string("C:\\Dev\\GD-Doom-Prototype");
    }

    std::string GetPrefPathInternal(const char *org, const char *app) override
    {
        return std::string("C:\\Dev\\GD-Doom-Prototype");
    }

    std::string GetEnvInternal(const char *name) override
    {
        return std::string("");
    }

    int OpenURLInternal(const char *url) override
    {
        return 0;
    }

    int ShowSimpleMessageBoxInternal(const char *title, const char *message) override
    {
        return 0;
    }

    void DelayInternal(uint32_t milliseconds) override
    {
        // SDL_Delay(milliseconds);
    }

    // Input

    void StartupControlInternal(void) override
    {
        alt_is_down = false;
    }

    void ShutdownControlInternal(void) override
    {
    }

    void ControlGetEventsInternal(void) override
    {
    }

    void CheckJoystickChangedInternal(void) override
    {
    }

    std::string JoystickNameForIndexInternal(int index) override
    {
        return std::string("None");
    }

    void SetRelativeMouseModeInternal(bool enabled) override
    {
    }

    // Video

    void *GetProgramWindowInternal() override
    {
        return nullptr;
    }

    void StartupGraphicsInternal() override
    {
    }

    void ShutdownGraphicsInternal(void) override
    {
    }

    bool InitializeWindowInternal(DisplayMode *mode) override
    {
        return true;
    }

    bool SetScreenSizeInternal(DisplayMode *mode) override
    {
        return true;
    }

    void StartFrameInternal(void) override
    {
    }

    void FinishFrameInternal(void) override
    {
    }

    void SwapBuffersInternal(void) override
    {
    }

  public:
    NullPlatform()
    {

        set(this);
    }

    virtual ~NullPlatform()
    {
    }
};

void Platform_Init()
{
    new NullPlatform();
}

} // namespace gd