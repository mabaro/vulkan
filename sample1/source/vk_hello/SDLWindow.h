#pragma once

#include <SDL2/SDL.h>
#include <cstdio>

class Window {
public:
    virtual const char* GetName() const = 0;

    virtual bool Init() = 0;
    virtual void Close() = 0;

    virtual void Draw() = 0;
    virtual void Run() = 0;

protected:
};

class SDLWindow : public Window {
protected:
    SDL_Window* _window = nullptr;
    SDL_Rect _screenRect = { 0, 0, 640, 480 };
    const char* _name;

public:
    SDLWindow(const char* name = "Undefined")
        : _name(name)
    {
    }

    const char* GetName() const override { return _name; }

    bool Init() override;
    void Close() override;

    void Draw() override;
    void Run() override;
};
