
#pragma once

#include <epi.h>
#include <stdint.h>

#include <string>

#include "../r_modes.h"

namespace gd
{

class Platform
{

  public:
    static uint32_t GetTicks()
    {
        CheckValid();

        return instance_->GetTicksInternal();
    }

    static std::string GetEnv(const char *name)
    {
        CheckValid();

        return instance_->GetEnvInternal(name);
    }

    static std::string GetBasePath()
    {
        CheckValid();

        return instance_->GetBasePathInternal();
    }

    static std::string GetPrefPath(const char *org, const char *app)
    {
        CheckValid();

        return instance_->GetPrefPathInternal(org, app);
    }

    static int OpenURL(const char *url)
    {
        CheckValid();

        return instance_->OpenURLInternal(url);
    }

    static void Delay(uint32_t milliseconds)
    {
        CheckValid();

        return instance_->DelayInternal(milliseconds);
    }

    // Input
    static void StartupControl()
    {
        CheckValid();

        instance_->StartupControlInternal();
    }

    static void ShutdownControl()
    {
        if (!instance_)
        {
            return;
        }

        instance_->ShutdownControlInternal();
    }

    static void ControlGetEvents()
    {
        CheckValid();

        instance_->ControlGetEventsInternal();
    }

    static void CheckJoystickChanged()
    {
        CheckValid();

        instance_->CheckJoystickChangedInternal();
    }

    static std::string JoystickNameForIndex(int index)
    {
        CheckValid();

        return instance_->JoystickNameForIndexInternal(index);
    }

    static void SetRelativeMouseMode(bool enabled)
    {
        CheckValid();

        instance_->SetRelativeMouseModeInternal(enabled);
    }

    // Video

    static void StartupGraphics()
    {
        CheckValid();

        instance_->StartupGraphicsInternal();
    }

    static bool InitializeWindow(DisplayMode *mode)
    {
        CheckValid();

        return instance_->InitializeWindowInternal(mode);
    }

    static bool SetScreenSize(DisplayMode *mode)
    {
        CheckValid();

        return instance_->SetScreenSizeInternal(mode);
    }

    static void *GetProgramWindow()
    {
        CheckValid();

        return instance_->GetProgramWindowInternal();
    }

    static void Shutdown()
    {
        if (instance_)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }

    static void StartFrame()
    {
        CheckValid();

        instance_->StartFrameInternal();
    }

    static void FinishFrame()
    {
        CheckValid();

        instance_->FinishFrameInternal();
    }

    static void ShutdownGraphics()
    {
        if (!instance_)
        {
            return;
        }

        instance_->ShutdownGraphicsInternal();
    }

    static void DebugPrint(const char* message)
    {
        if (!instance_)
        {
            return;
        }

        instance_->DebugPrintInternal(message);
    }


  protected:
    virtual ~Platform()
    {
    }

    virtual std::string GetBasePathInternal(void) = 0;

    virtual std::string GetPrefPathInternal(const char *org, const char *app) = 0;

    virtual std::string GetEnvInternal(const char *name) = 0;

    virtual uint32_t GetTicksInternal() = 0;

    virtual int OpenURLInternal(const char *url) = 0;

    virtual void DelayInternal(uint32_t milliseconds) = 0;

    // Input
    virtual void StartupControlInternal()  = 0;
    virtual void ShutdownControlInternal() = 0;

    virtual void ControlGetEventsInternal() = 0;

    virtual void CheckJoystickChangedInternal() = 0;

    virtual std::string JoystickNameForIndexInternal(int index) = 0;

    virtual void SetRelativeMouseModeInternal(bool enabled) = 0;

    // Video

    virtual bool InitializeWindowInternal(DisplayMode *mode) = 0;

    virtual bool SetScreenSizeInternal(DisplayMode *mode) = 0;

    virtual void *GetProgramWindowInternal() = 0;

    virtual void StartupGraphicsInternal() = 0;

    virtual void ShutdownGraphicsInternal(void) = 0;

    virtual void StartFrameInternal(void) = 0;

    virtual void FinishFrameInternal(void) = 0;

    virtual void SwapBuffersInternal(void) = 0;

    virtual void DebugPrintInternal(const char* message)
    {
        EPI_UNUSED(message);
    }

    static void set(Platform *platform);    

    static void CheckValid()
    {
        if (!instance_)
        {
            FatalError("Platform: Not Initialized\n");
        }
    }

    static Platform *instance_;
};

void Platform_Init();
void Platform_Shutdown();
// This may only exist as long as SDL is still in, but this is outside of a specific
// instance because it does not require a valid platform instance and is thus suitable
// for things like FatalError - Dasho
void Platform_SimpleMessageBox(const char *title, const char *message);

} // namespace gd
