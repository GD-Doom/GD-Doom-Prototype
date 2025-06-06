
#pragma once

#include <epi.h>

#include <string>

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

        return instance_->OpenURL(url);
    }

    static void shutdown()
    {
        if (instance_)
        {
            delete instance_;
            instance_ = nullptr;
        }
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

} // namespace gd