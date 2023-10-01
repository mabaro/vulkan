#include "SDLWindow.h"

#include "core/core.h"

#include <backends/imgui_impl_sdl2.h>

////////////////////////////////////////////////////////////////////////////////

bool
SDLWindow::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        SDL_Log("Warning: Linear texture filtering not enabled!");
    }

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        _screenRect.w = displayMode.w >> 1;
        _screenRect.h = displayMode.h >> 1;
    }

    const uint32_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    _window                    = SDL_CreateWindow("Vulkan Hello", SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,        // window pos
                           _screenRect.w, _screenRect.h,   // window size
                           windowFlags);
    if (_window == NULL) {
        SDL_Log("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (false) {   // workaround to show the window
        SDL_Renderer* renderer = SDL_CreateRenderer(_window, -1, 0);
        SDL_RenderPresent(renderer);
        SDL_DestroyRenderer(renderer);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindow::Close()
{
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = NULL;
    }

    SDL_Quit();
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindow::_DrawFrame()
{
    // nothing
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindow::_OnMainLoopExit()
{
    // nothing
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindow::_OnResize(uint32_t /* width */, uint32_t /* height */)
{
    // nothing
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindow::Run()
{
    ASSERT(_window);
    if (_window == nullptr) {
        return;
    }

    const int kMinSizeToDraw = 1;

    SDL_Event e;
    bool      quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            // printf("Event: %d\n", e.type);
            if (e.type == SDL_JOYBUTTONDOWN) {
                // SDL_JOYAXISMOTION = 0x600, /**< Joystick axis motion */
                // SDL_JOYBALLMOTION,     /**< Joystick trackball motion */
                // SDL_JOYHATMOTION,      /**< Joystick hat position change */
                // SDL_JOYBUTTONDOWN,     /**< Joystick button pressed */
                // SDL_JOYBUTTONUP,       /**< Joystick button released */
                // SDL_JOYDEVICEADDED,    /**< A new joystick has been inserted
                // into the system */ SDL_JOYDEVICEREMOVED,  /**< An opened
                // joystick has been removed */
            } else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                switch (e.type) {
                case SDL_MOUSEMOTION:
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    break;
                case SDL_MOUSEBUTTONUP:
                    break;
                }
            } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                // const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
                // if (currentKeyStates[SDL_SCANCODE_UP])
                // {
                // }

                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (e.key.keysym.sym == SDLK_a) {
                    // SDL_MinimizeWindow(_window);
                    SDL_SetWindowSize(_window, 0, 0);
                }
            } else if (e.type == SDL_WINDOWEVENT) {
                switch (e.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    _OnResize(e.window.data1, e.window.data2);
                    _shouldRender = e.window.data1 > kMinSizeToDraw && e.window.data2 > kMinSizeToDraw;
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    _OnResize(e.window.data1, e.window.data2);
                    _shouldRender = e.window.data1 > kMinSizeToDraw && e.window.data2 > kMinSizeToDraw;
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    _shouldRender = false;
                    break;
                case SDL_WINDOWEVENT_SHOWN:
                    _shouldRender = true;
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    _shouldRender = false;
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    break;
                case SDL_WINDOWEVENT_TAKE_FOCUS:
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    break;
                }
            } else if (e.type == SDL_QUIT) {
                quit = true;
            } else {
                LOG_DEBUG("unhandled event: ", e.type);
            }
        }

        if (_shouldRender) {
            _DrawFrame();
        }
    }

    _OnMainLoopExit();
}

////////////////////////////////////////////////////////////////////////////////

void
Window::_OnResize(uint32_t /*width*/, uint32_t /*height*/)
{
    // nothing
}
