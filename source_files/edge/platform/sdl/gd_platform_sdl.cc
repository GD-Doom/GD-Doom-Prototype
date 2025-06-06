
#include <SDL2/SDL.h>

#include "../gd_platform.h"

namespace gd
{

class SDL_Platform : public Platform
{

  protected:
    std::string GetBasePathInternal(void) override
    {
        const char *path = SDL_GetBasePath();

        std::string result(path);

        SDL_free((void *)path);

        return result;
    }

    std::string GetPrefPathInternal(const char *org, const char *app) override
    {
        const char *path = SDL_GetPrefPath(org, app);

        std::string result(path);

        SDL_free((void *)path);

        return result;
    }

  public:
    SDL_Platform()
    {
        set(this);
    }
};

void Platform_Init()
{
    new SDL_Platform();
}

} // namespace gd