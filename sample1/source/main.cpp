// tuto: https://lazyfoo.net/tutorials/SDL/52_hello_mobile/android_linux/index.php
// APK from linux : https://authmane512.medium.com/how-to-build-an-apk-from-command-line-without-ide-7260e1e22676
// gdb on android: https://gist.github.com/sekkr1/6adf2741ed3bc741b53ab276d35fd047 / https://www.amongbytes.com/post/201804-debugging-on-android/

#include <SDL2/SDL.h>
#include <cstdio>

SDL_Window *gWindow = NULL;
SDL_Rect gScreenRect = {0, 0, 640, 480};
SDL_Renderer* gRenderer = NULL;

////////////////////////////////////////////////////////////////////////////////

bool Init()
{
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		SDL_Log( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        return false;
    }

    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
    {
        SDL_Log( "Warning: Linear texture filtering not enabled!" );
    }

    SDL_DisplayMode displayMode;
    if( SDL_GetCurrentDisplayMode( 0, &displayMode ) == 0 )
    {
        gScreenRect.w = displayMode.w;
        gScreenRect.h = displayMode.h;
    }

    gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenRect.w, gScreenRect.h, SDL_WINDOW_SHOWN );
    if( gWindow == NULL )
    {
        SDL_Log( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
        return false;
    }

    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( gRenderer == NULL )
    {
        SDL_Log( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
        return false;
    }

    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

	return true;
}

////////////////////////////////////////////////////////////////////////////////

void Close()
{
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

    SDL_Quit();
}

////////////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char ** /*argv*/)
{
    if (Init())
    {
        // gScreenSurface = SDL_GetWindowSurface(gWindow);
        // SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(gScreenSurface->format, 0xFF, 0xFF, 0xFF));
        // SDL_UpdateWindowSurface(gWindow);

        SDL_Event e;
        bool quit = false;
        while (!quit)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                    quit = true;
            }

            SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(gRenderer);


            SDL_RenderPresent(gRenderer);
        }
    }

    Close();

    return 0;
}
