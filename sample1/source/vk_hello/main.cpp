// https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp
#include "SDLWindowVulkan.h"

////////////////////////////////////////////////////////////////////////////////

int
main(int /* argc */, char** /* argv */)
{
    SDLWindowVulkan window;
    if (window.Init()) {
        window.Run();
        window.Close();
    }

    return 0;
}
