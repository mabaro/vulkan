#pragma once

#include <SDL2/SDL.h>

class Window {
public:
    virtual const char* GetName() const = 0;

    virtual bool Init()  = 0;
    virtual void Close() = 0;

    virtual void Run()       = 0;

protected:
    virtual void _DrawFrame() = 0;

    //! Will be called when the main loop is finished
    //! in order to wait for any ongoing async processes
    virtual void _OnMainLoopExit() = 0;

    virtual void _OnResize(uint32_t width, uint32_t height) = 0;
};

class SDLWindow : public Window {
protected:
    SDL_Window* _window     = nullptr;
    SDL_Rect    _screenRect = {0, 0, 640, 480};
    const char* _name;

public:
    SDLWindow(const char* name = "Undefined")
        : _name(name)
    {
    }

    const char* GetName() const override { return _name; }

    bool Init() override;
    void Close() override;

    void Run() override;

protected:
    void _DrawFrame() override;
    void _OnMainLoopExit() override;

    void _OnResize(uint32_t width, uint32_t height) override;
};
