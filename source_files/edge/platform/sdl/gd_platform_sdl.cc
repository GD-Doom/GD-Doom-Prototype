
#include <SDL2/SDL.h>
#include <dm_defs.h>
#include <epi_str_util.h>

#include "../../e_input.h"
#include "../../e_main.h"
#include "../../i_system.h"
#include "../../m_argv.h"
#include "../../r_modes.h"
#include "../gd_platform.h"

bool need_mouse_recapture = false;

bool       no_joystick;
int        joystick_device;  // choice in menu, 0 for none
static int total_joysticks;
static int current_joystick; // 0 for none

static SDL_Joystick       *joystick_info = nullptr;
static SDL_GameController *gamepad_info  = nullptr;

extern bool eat_mouse_motion;
void        HandleFocusLost(void);
void        HandleFocusGain(void);

void ShowGamepads(void)
{
    if (no_joystick)
    {
        LogPrint("Gamepad system is disabled.\n");
        return;
    }

    if (total_joysticks == 0)
    {
        LogPrint("No gamepads found.\n");
        return;
    }

    LogPrint("Gamepads:\n");

    for (int i = 0; i < total_joysticks; i++)
    {
        const char *name = SDL_GameControllerNameForIndex(i);
        if (!name)
            name = "(UNKNOWN)";

        LogPrint("  %2d : %s\n", i + 1, name);
    }
}

int JoystickGetAxis(int n) // n begins at 0
{
    if (no_joystick || !joystick_info || !gamepad_info)
        return 0;

    return SDL_GameControllerGetAxis(gamepad_info, (SDL_GameControllerAxis)n);
}

namespace gd
{

static SDL_JoystickID current_gamepad = -1;

static int  TranslateSDLKey(SDL_Scancode key);
static void StartupJoystick(void);
static void ActiveEventProcess(SDL_Event *sdl_ev);
static void InactiveEventProcess(SDL_Event *sdl_ev);
static void I_OpenJoystick(int index);

bool alt_is_down;

// Track trigger state to avoid pushing multiple unnecessary trigger events
bool right_trigger_pulled = false;
bool left_trigger_pulled  = false;

class SDL_Platform : public Platform
{

  protected:
    uint32_t GetTicksInternal() override
    {
        return SDL_GetTicks();
    }

    std::string GetBasePathInternal(void) override
    {
        const char *path = SDL_GetBasePath();

        std::string result(path ? path : "");

        SDL_free((void *)path);

        return result;
    }

    std::string GetPrefPathInternal(const char *org, const char *app) override
    {
        const char *path = SDL_GetPrefPath(org, app);

        std::string result(path ? path : "");

        SDL_free((void *)path);

        return result;
    }

    std::string GetEnvInternal(const char *name) override
    {
        const char *value = SDL_getenv(name);

        std::string result(value ? value : "");

        SDL_free((void *)value);

        return result;
    }

    int OpenURLInternal(const char *url) override
    {
        return SDL_OpenURL(url);
    }

    int ShowSimpleMessageBoxInternal(const char *title, const char *message) override
    {
        return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, nullptr);
    }

    void DelayInternal(uint32_t milliseconds) override
    {
        SDL_Delay(milliseconds);
    }

    // Input

    void StartupControlInternal(void) override
    {
        alt_is_down = false;
        StartupJoystick();
    }

    void ShutdownControlInternal(void) override
    {
        if (SDL_WasInit(SDL_INIT_GAMECONTROLLER))
        {
            SDL_GameControllerEventState(SDL_IGNORE);
            if (gamepad_info)
            {
                SDL_GameControllerClose(gamepad_info);
                gamepad_info = nullptr;
            }
            if (joystick_info)
            {
                SDL_JoystickClose(joystick_info);
                joystick_info = nullptr;
            }
            SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        }
    }

    void ControlGetEventsInternal(void) override
    {
        SDL_Event sdl_ev;

        while (SDL_PollEvent(&sdl_ev))
        {
            if (app_state & kApplicationActive)
                ActiveEventProcess(&sdl_ev);
            else
                InactiveEventProcess(&sdl_ev);
        }
    }

    void CheckJoystickChangedInternal(void) override
    {
        int new_total_joysticks = SDL_NumJoysticks();

        if (new_total_joysticks == total_joysticks && current_joystick == joystick_device)
            return;

        if (new_total_joysticks == 0)
        {
            if (gamepad_info)
            {
                SDL_GameControllerClose(gamepad_info);
                gamepad_info = nullptr;
            }
            if (joystick_info)
            {
                SDL_JoystickClose(joystick_info);
                joystick_info = nullptr;
            }
            total_joysticks  = 0;
            joystick_device  = 0;
            current_joystick = 0;
            current_gamepad  = -1;
            return;
        }

        int new_joy = joystick_device;

        if (joystick_device < 0 || joystick_device > new_total_joysticks)
        {
            joystick_device = 0;
            new_joy         = 0;
        }

        if (new_joy == current_joystick && current_joystick > 0)
            return;

        if (joystick_info)
        {
            if (gamepad_info)
            {
                SDL_GameControllerClose(gamepad_info);
                gamepad_info = nullptr;
            }

            SDL_JoystickClose(joystick_info);
            joystick_info = nullptr;

            LogPrint("Closed joystick %d\n", current_joystick);
            current_joystick = 0;

            current_gamepad = -1;
        }

        if (new_joy > 0)
        {
            total_joysticks = new_total_joysticks;
            joystick_device = new_joy;
            I_OpenJoystick(new_joy);
        }
        else if (total_joysticks == 0 && new_total_joysticks > 0)
        {
            total_joysticks = new_total_joysticks;
            joystick_device = new_joy = 1;
            I_OpenJoystick(new_joy);
        }
        else
            total_joysticks = new_total_joysticks;
    }

    std::string JoystickNameForIndexInternal(int index) override
    {
        std::string result;
        const char *joyname = SDL_JoystickNameForIndex(index - 1);
        if (joyname)
        {
            result = joyname;
        }

        return result;
    }

  public:
    SDL_Platform()
    {
#ifdef EDGE_MIMALLOC
        if (SDL_SetMemoryFunctions(mi_malloc, mi_calloc, mi_realloc, mi_free) != 0)
            FatalError("Couldn't init SDL memory functions!!\n%s\n", SDL_GetError());
#endif
        if (SDL_Init(0) < 0)
            FatalError("Couldn't init SDL!!\n%s\n", SDL_GetError());

        set(this);
    }

    virtual ~SDL_Platform()
    {
        SDL_Quit();
    }
};

void Platform_Init()
{
    new SDL_Platform();
}

// Input
//
// Translates a key from SDL -> EDGE
// Returns -1 if no suitable translation exists.
//
static int TranslateSDLKey(SDL_Scancode key)
{
    switch (key)
    {
    case SDL_SCANCODE_GRAVE:
        return kTilde;
    case SDL_SCANCODE_MINUS:
        return kMinus;
    case SDL_SCANCODE_EQUALS:
        return kEquals;

    case SDL_SCANCODE_TAB:
        return kTab;
    case SDL_SCANCODE_RETURN:
        return kEnter;
    case SDL_SCANCODE_ESCAPE:
        return kEscape;
    case SDL_SCANCODE_BACKSPACE:
        return kBackspace;

    case SDL_SCANCODE_UP:
        return kUpArrow;
    case SDL_SCANCODE_DOWN:
        return kDownArrow;
    case SDL_SCANCODE_LEFT:
        return kLeftArrow;
    case SDL_SCANCODE_RIGHT:
        return kRightArrow;

    case SDL_SCANCODE_HOME:
        return kHome;
    case SDL_SCANCODE_END:
        return kEnd;
    case SDL_SCANCODE_INSERT:
        return kInsert;
    case SDL_SCANCODE_DELETE:
        return kDelete;
    case SDL_SCANCODE_PAGEUP:
        return kPageUp;
    case SDL_SCANCODE_PAGEDOWN:
        return kPageDown;

    case SDL_SCANCODE_F1:
        return kFunction1;
    case SDL_SCANCODE_F2:
        return kFunction2;
    case SDL_SCANCODE_F3:
        return kFunction3;
    case SDL_SCANCODE_F4:
        return kFunction4;
    case SDL_SCANCODE_F5:
        return kFunction5;
    case SDL_SCANCODE_F6:
        return kFunction6;
    case SDL_SCANCODE_F7:
        return kFunction7;
    case SDL_SCANCODE_F8:
        return kFunction8;
    case SDL_SCANCODE_F9:
        return kFunction9;
    case SDL_SCANCODE_F10:
        return kFunction10;
    case SDL_SCANCODE_F11:
        return kFunction11;
    case SDL_SCANCODE_F12:
        return kFunction12;

    case SDL_SCANCODE_KP_0:
        return kKeypad0;
    case SDL_SCANCODE_KP_1:
        return kKeypad1;
    case SDL_SCANCODE_KP_2:
        return kKeypad2;
    case SDL_SCANCODE_KP_3:
        return kKeypad3;
    case SDL_SCANCODE_KP_4:
        return kKeypad4;
    case SDL_SCANCODE_KP_5:
        return kKeypad5;
    case SDL_SCANCODE_KP_6:
        return kKeypad6;
    case SDL_SCANCODE_KP_7:
        return kKeypad7;
    case SDL_SCANCODE_KP_8:
        return kKeypad8;
    case SDL_SCANCODE_KP_9:
        return kKeypad9;

    case SDL_SCANCODE_KP_PERIOD:
        return kKeypadDot;
    case SDL_SCANCODE_KP_PLUS:
        return kKeypadPlus;
    case SDL_SCANCODE_KP_MINUS:
        return kKeypadMinus;
    case SDL_SCANCODE_KP_MULTIPLY:
        return kKeypadStar;
    case SDL_SCANCODE_KP_DIVIDE:
        return kKeypadSlash;
    case SDL_SCANCODE_KP_EQUALS:
        return kKeypadEquals;
    case SDL_SCANCODE_KP_ENTER:
        return kKeypadEnter;

    case SDL_SCANCODE_PRINTSCREEN:
        return kPrintScreen;
    case SDL_SCANCODE_CAPSLOCK:
        return kCapsLock;
    case SDL_SCANCODE_NUMLOCKCLEAR:
        return kNumberLock;
    case SDL_SCANCODE_SCROLLLOCK:
        return kScrollLock;
    case SDL_SCANCODE_PAUSE:
        return kPause;

    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:
        return kRightShift;
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL:
        return kRightControl;
    case SDL_SCANCODE_LGUI:
    case SDL_SCANCODE_LALT:
        return kLeftAlt;
    case SDL_SCANCODE_RGUI:
    case SDL_SCANCODE_RALT:
        return kRightAlt;

    default:
        break;
    }

    if (key <= 0x7f)
        return epi::ToLowerASCII(SDL_GetKeyFromScancode(key));

    return -1;
}

static void HandleKeyEvent(SDL_Event *ev)
{
    if (ev->type != SDL_KEYDOWN && ev->type != SDL_KEYUP)
        return;

    SDL_Scancode sym = ev->key.keysym.scancode;

    InputEvent event;
    event.modstate      = kInputEventModNone;
    event.value.key.sym = TranslateSDLKey(sym);

    SDL_Keymod mod = SDL_GetModState();
    if (mod & KMOD_SHIFT)
    {
        event.modstate = kInputEventModLeftShift;
    }
    else if (mod & KMOD_CAPS)
    {
        event.modstate = kInputEventModCaps;
    }

    // handle certain keys which don't behave normally
    if (sym == SDL_SCANCODE_CAPSLOCK || sym == SDL_SCANCODE_NUMLOCKCLEAR)
    {
        if (ev->type != SDL_KEYDOWN)
            return;
        event.type = kInputEventKeyDown;
        PostEvent(&event);

        event.type = kInputEventKeyUp;
        PostEvent(&event);
        return;
    }

    event.type = (ev->type == SDL_KEYDOWN) ? kInputEventKeyDown : kInputEventKeyUp;

    if (event.value.key.sym < 0)
    {
        // No translation possible for SDL symbol and no unicode value
        return;
    }

    if (event.value.key.sym == kTab && alt_is_down)
    {
        alt_is_down = false;
        return;
    }

    if (event.value.key.sym == kEnter && alt_is_down)
    {
        alt_is_down = false;
        ToggleFullscreen();
        if (current_window_mode == kWindowModeWindowed)
        {
            GrabCursor(false);
        }
        return;
    }

    if (event.value.key.sym == kLeftAlt)
        alt_is_down = (event.type == kInputEventKeyDown);

    PostEvent(&event);
}

static void HandleMouseButtonEvent(SDL_Event *ev)
{
    InputEvent event;
    event.modstate = kInputEventModNone;

    if (ev->type == SDL_MOUSEBUTTONDOWN)
        event.type = kInputEventKeyDown;
    else if (ev->type == SDL_MOUSEBUTTONUP)
        event.type = kInputEventKeyUp;
    else
        return;

    switch (ev->button.button)
    {
    case 1:
        event.value.key.sym = kMouse1;
        break;
    case 2:
        event.value.key.sym = kMouse2;
        break;
    case 3:
        event.value.key.sym = kMouse3;
        break;
    case 4:
        event.value.key.sym = kMouse4;
        break;
    case 5:
        event.value.key.sym = kMouse5;
        break;
    case 6:
        event.value.key.sym = kMouse6;
        break;

    default:
        return;
    }

    PostEvent(&event);
}

static void HandleMouseWheelEvent(SDL_Event *ev)
{
    InputEvent event;
    event.modstate = kInputEventModNone;
    InputEvent release;
    release.modstate = kInputEventModNone;

    event.type   = kInputEventKeyDown;
    release.type = kInputEventKeyUp;

    if (ev->wheel.y > 0)
    {
        event.value.key.sym   = kMouseWheelUp;
        release.value.key.sym = kMouseWheelUp;
    }
    else if (ev->wheel.y < 0)
    {
        event.value.key.sym   = kMouseWheelDown;
        release.value.key.sym = kMouseWheelDown;
    }
    else
    {
        return;
    }
    PostEvent(&event);
    PostEvent(&release);
}

static void HandleGamepadButtonEvent(SDL_Event *ev)
{
    // ignore other gamepads;
    if (ev->cbutton.which != current_gamepad)
        return;

    InputEvent event;
    event.modstate = kInputEventModNone;

    if (ev->type == SDL_CONTROLLERBUTTONDOWN)
        event.type = kInputEventKeyDown;
    else if (ev->type == SDL_CONTROLLERBUTTONUP)
        event.type = kInputEventKeyUp;
    else
        return;

    if (ev->cbutton.button >= SDL_CONTROLLER_BUTTON_MAX) // How would this happen? - Dasho
        return;

    event.value.key.sym = kGamepadA + ev->cbutton.button;

    PostEvent(&event);
}

static void HandleGamepadTriggerEvent(SDL_Event *ev)
{
    // ignore other gamepads
    if (ev->caxis.which != current_gamepad)
        return;

    Uint8 current_axis = ev->caxis.axis;

    // ignore non-trigger axes
    if (current_axis != SDL_CONTROLLER_AXIS_TRIGGERLEFT && current_axis != SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        return;

    InputEvent event;
    event.modstate = kInputEventModNone;

    int thresh = RoundToInteger(*joystick_deadzones[current_axis] * 32767.0f);
    int input  = ev->caxis.value;

    if (current_axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
    {
        event.value.key.sym = kGamepadTriggerLeft;
        if (input < thresh)
        {
            if (!left_trigger_pulled)
                return;
            event.type          = kInputEventKeyUp;
            left_trigger_pulled = false;
        }
        else
        {
            if (left_trigger_pulled)
                return;
            event.type          = kInputEventKeyDown;
            left_trigger_pulled = true;
        }
    }
    else
    {
        event.value.key.sym = kGamepadTriggerRight;
        if (input < thresh)
        {
            if (!right_trigger_pulled)
                return;
            event.type           = kInputEventKeyUp;
            right_trigger_pulled = false;
        }
        else
        {
            if (right_trigger_pulled)
                return;
            event.type           = kInputEventKeyDown;
            right_trigger_pulled = true;
        }
    }

    PostEvent(&event);
}

static void HandleMouseMotionEvent(SDL_Event *ev)
{
    int dx, dy;

    dx = ev->motion.xrel;
    dy = ev->motion.yrel;

    if (dx || dy)
    {
        InputEvent event;
        event.modstate = kInputEventModNone;

        event.type           = kInputEventKeyMouse;
        event.value.mouse.dx = dx;
        event.value.mouse.dy = -dy; // -AJA- positive should be "up"

        PostEvent(&event);
    }
}

static void I_OpenJoystick(int index)
{
    EPI_ASSERT(1 <= index && index <= total_joysticks);

    joystick_info = SDL_JoystickOpen(index - 1);
    if (!joystick_info)
    {
        LogPrint("Unable to open joystick %d (SDL error)\n", index);
        return;
    }

    current_joystick = index;

    gamepad_info = SDL_GameControllerOpen(current_joystick - 1);

    if (!gamepad_info)
    {
        LogPrint("Unable to open joystick %s as a gamepad!\n", SDL_JoystickName(joystick_info));
        SDL_JoystickClose(joystick_info);
        joystick_info = nullptr;
        return;
    }

    current_gamepad = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_info));

    const char *name = SDL_GameControllerName(gamepad_info);
    if (!name)
        name = "(UNKNOWN)";

    int gp_total_joysticksticks = 0;
    int gp_num_triggers         = 0;
    int gp_num_buttons          = 0;

    if (SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_LEFTX) &&
        SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_LEFTY))
        gp_total_joysticksticks++;
    if (SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_RIGHTX) &&
        SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_RIGHTY))
        gp_total_joysticksticks++;
    if (SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_TRIGGERLEFT))
        gp_num_triggers++;
    if (SDL_GameControllerHasAxis(gamepad_info, SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
        gp_num_triggers++;
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        if (SDL_GameControllerHasButton(gamepad_info, (SDL_GameControllerButton)i))
            gp_num_buttons++;
    }

    LogPrint("Opened gamepad %d : %s\n", current_joystick, name);
    LogPrint("Sticks:%d Triggers: %d Buttons: %d Touchpads: %d\n", gp_total_joysticksticks, gp_num_triggers,
             gp_num_buttons, SDL_GameControllerGetNumTouchpads(gamepad_info));
    LogPrint("Rumble:%s Trigger Rumble: %s LED: %s\n", SDL_GameControllerHasRumble(gamepad_info) ? "Yes" : "No",
             SDL_GameControllerHasRumbleTriggers(gamepad_info) ? "Yes" : "No",
             SDL_GameControllerHasLED(gamepad_info) ? "Yes" : "No");
}
//
// Event handling while the application is active
//
static void ActiveEventProcess(SDL_Event *sdl_ev)
{
    switch (sdl_ev->type)
    {
    case SDL_WINDOWEVENT: {
        if (sdl_ev->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            HandleFocusLost();
        }

        break;
    }

    case SDL_KEYDOWN:
    case SDL_KEYUP:
        HandleKeyEvent(sdl_ev);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if (need_mouse_recapture)
        {
            GrabCursor(true);
            break;
        }
        HandleMouseButtonEvent(sdl_ev);
        break;

    case SDL_MOUSEWHEEL:
        if (!need_mouse_recapture)
            HandleMouseWheelEvent(sdl_ev);
        break;

    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        HandleGamepadButtonEvent(sdl_ev);
        break;

    // Analog triggers should be the only thing handled here -Dasho
    case SDL_CONTROLLERAXISMOTION:
        HandleGamepadTriggerEvent(sdl_ev);
        break;

    case SDL_MOUSEMOTION:
        if (eat_mouse_motion)
        {
            eat_mouse_motion = false; // One motion needs to be discarded
            break;
        }
        if (!need_mouse_recapture)
            HandleMouseMotionEvent(sdl_ev);
        break;

    case SDL_QUIT:
        // Note we deliberate clear all other flags here. Its our method of
        // ensuring nothing more is done with events.
        app_state = kApplicationPendingQuit;
        break;

    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
        Platform::CheckJoystickChanged();
        break;

    default:
        break; // Don't care
    }
}

//
// Event handling while the application is not active
//
static void InactiveEventProcess(SDL_Event *sdl_ev)
{
    switch (sdl_ev->type)
    {
    case SDL_WINDOWEVENT:
        if (app_state & kApplicationPendingQuit)
            break; // Don't care: we're going to exit

        if (sdl_ev->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
            HandleFocusGain();
        break;

    case SDL_QUIT:
        // Note we deliberate clear all other flags here. Its our method of
        // ensuring nothing more is done with events.
        app_state = kApplicationPendingQuit;
        break;

    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
        Platform::CheckJoystickChanged();
        break;

    default:
        break; // Don't care
    }
}

void StartupJoystick(void)
{
    current_joystick = 0;
    joystick_device  = 0;

    if (FindArgument("no_joystick") > 0)
    {
        LogPrint("StartupControl: Gamepad system disabled.\n");
        no_joystick = true;
        return;
    }

    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
    {
        LogPrint("StartupControl: Couldn't init SDL GAMEPAD!\n");
        no_joystick = true;
        return;
    }

    SDL_GameControllerEventState(SDL_ENABLE);

    total_joysticks = SDL_NumJoysticks();

    LogPrint("StartupControl: %d gamepads found.\n", total_joysticks);

    if (total_joysticks == 0)
        return;
    else
    {
        joystick_device = 1; // Automatically set to first detected gamepad
        I_OpenJoystick(joystick_device);
    }
}

} // namespace gd