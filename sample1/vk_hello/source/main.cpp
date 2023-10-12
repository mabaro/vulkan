// https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp
#include "SDLWindowVulkan.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <tchar.h>
int _tmain(int , TCHAR**)
#else
int main(int /*argc*/, char** /*argv*/)
#endif // ABC_PLATFORM_WINDOWS_FAMILY
{
    gfx::SDLWindowVulkan window;
    if (window.Init()) {
        window.Run();
        window.Close();
    }

    return 0;
}
