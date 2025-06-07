
#include "gd_platform.h"

namespace gd
{

Platform *Platform::instance_ = nullptr;

void Platform::set(Platform *platform)
{
    instance_ = platform;
}

void Platform_Shutdown()
{
    Platform::Shutdown();
}

} // namespace gd