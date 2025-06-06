
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
};

void Platform_Init()
{
    new SDL_Platform();
}

} // namespace gd