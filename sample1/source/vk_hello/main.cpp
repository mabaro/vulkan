#include "SDLWindow.h"
#include "gfx/Device.h"

int
main(int /* argc */, char** /* argv */) {
    SDLWindow window;
    if (window.Init()) {
        window.Run();
        window.Close();
    }

    return 0;
}
