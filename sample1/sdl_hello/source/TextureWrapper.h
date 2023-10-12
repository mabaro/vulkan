#include <SDL.h>
#include <string>


class SDLTextureWrapper
{
public:
    SDLTextureWrapper(SDL_Window* gWindow, SDL_Renderer *gRenderer);
    ~SDLTextureWrapper();

    bool loadFromFile(std::string path);
    bool loadPixelsFromFile(std::string path);
    bool loadFromPixels();

#if defined(SDL_TTF_MAJOR_VERSION)
    // Creates image from font string
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor;
#endif

    bool createBlank(int width, int height, SDL_TextureAccess access);

    void free();

    void setColor(Uint8 red, Uint8 green, Uint8 blue);
    void setBlendMode(SDL_BlendMode blending);
    void setAlpha(Uint8 alpha);
    void render(int x, int y, SDL_Rect *clip = NULL, double angle = 0.0, SDL_Point *center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

    int getWidth();
    int getHeight();
    Uint32 *getPixels32();
    Uint32 getPixel32(Uint32 x, Uint32 y);
    Uint32 getPitch32();

    void copyRawPixels32(void *pixels);
    bool lockTexture();
    bool unlockTexture();

private:
    SDL_Window* gWindow;
    SDL_Renderer *gRenderer;
    SDL_Texture *mTexture;
    SDL_Surface *mSurfacePixels;

    void *mRawPixels;
    int mRawPitch;
    int mWidth;
    int mHeight;
};

SDLTextureWrapper::SDLTextureWrapper(SDL_Window* i_gWindow, SDL_Renderer *i_gRenderer)
{
    gWindow = i_gWindow;
    gRenderer = i_gRenderer;
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;

    mSurfacePixels = NULL;
    mRawPixels = NULL;
    mRawPitch = 0;
}

SDLTextureWrapper::~SDLTextureWrapper()
{
    free();
}

bool SDLTextureWrapper::loadFromFile(std::string path)
{
    if (!loadPixelsFromFile(path))
    {
        SDL_Log("Failed to load pixels for %s!\n", path.c_str());
    }
    else
    {
        if (!loadFromPixels())
        {
            SDL_Log("Failed to texture from pixels from %s!\n", path.c_str());
        }
    }

    return mTexture != NULL;
}

bool SDLTextureWrapper::loadPixelsFromFile(std::string path)
{
    free();

    SDL_Surface *loadedSurface = SDL_LoadBMP(path.c_str());
    if (loadedSurface == NULL)
    {
        SDL_Log("Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
    }
    else
    {
        const auto format = SDL_GetWindowPixelFormat(gWindow);
        mSurfacePixels = SDL_ConvertSurfaceFormat(loadedSurface, format, 0);
        if (mSurfacePixels == NULL)
        {
            SDL_Log("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            mWidth = mSurfacePixels->w;
            mHeight = mSurfacePixels->h;
        }

        SDL_FreeSurface(loadedSurface);
    }

    return mSurfacePixels != NULL;
}

bool SDLTextureWrapper::loadFromPixels()
{
    if (mSurfacePixels == NULL)
    {
        SDL_Log("No pixels loaded!");
    }
    else
    {
        SDL_SetColorKey(mSurfacePixels, SDL_TRUE, SDL_MapRGB(mSurfacePixels->format, 0, 0xFF, 0xFF));
        mTexture = SDL_CreateTextureFromSurface(gRenderer, mSurfacePixels);
        if (mTexture == NULL)
        {
            SDL_Log("Unable to create texture from loaded pixels! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            mWidth = mSurfacePixels->w;
            mHeight = mSurfacePixels->h;
        }

        SDL_FreeSurface(mSurfacePixels);
        mSurfacePixels = NULL;
    }

    return mTexture != NULL;
}

#if defined(SDL_TTF_MAJOR_VERSION)
bool SDLTextureWrapper::loadFromRenderedText(std::string textureText, SDL_Color textColor
{
    free();

    SDL_Surface *textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface != NULL)
    {
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture == NULL)
        {
            SDL_Log("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        SDL_FreeSurface(textSurface);
    }
    else
    {
        SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
    }

    return mTexture != NULL;
}
#endif

bool SDLTextureWrapper::createBlank(int width, int height, SDL_TextureAccess access)
{
    free();

    mTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);
    if (mTexture == NULL)
    {
        SDL_Log("Unable to create streamable blank texture! SDL Error: %s\n", SDL_GetError());
    }
    else
    {
        mWidth = width;
        mHeight = height;
    }

    return mTexture != NULL;
}

void SDLTextureWrapper::free()
{
    if (mTexture != NULL)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }

    if (mSurfacePixels != NULL)
    {
        SDL_FreeSurface(mSurfacePixels);
        mSurfacePixels = NULL;
    }
}

void SDLTextureWrapper::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void SDLTextureWrapper::setBlendMode(SDL_BlendMode blending)
{
    SDL_SetTextureBlendMode(mTexture, blending);
}

void SDLTextureWrapper::setAlpha(Uint8 alpha)
{
    SDL_SetTextureAlphaMod(mTexture, alpha);
}

void SDLTextureWrapper::render(int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};

    if (clip != NULL)
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int SDLTextureWrapper::getWidth()
{
    return mWidth;
}

int SDLTextureWrapper::getHeight()
{
    return mHeight;
}

Uint32 *SDLTextureWrapper::getPixels32()
{
    Uint32 *pixels = NULL;

    if (mSurfacePixels != NULL)
    {
        pixels = static_cast<Uint32 *>(mSurfacePixels->pixels);
    }

    return pixels;
}

Uint32 SDLTextureWrapper::getPixel32(Uint32 x, Uint32 y)
{
    Uint32 *pixels = static_cast<Uint32 *>(mSurfacePixels->pixels);
    return pixels[(y * getPitch32()) + x];
}

Uint32 SDLTextureWrapper::getPitch32()
{
    Uint32 pitch = 0;

    if (mSurfacePixels != NULL)
    {
        pitch = mSurfacePixels->pitch / 4;
    }

    return pitch;
}

bool SDLTextureWrapper::lockTexture()
{
    bool success = true;

    if (mRawPixels != NULL)
    {
        SDL_Log("Texture is already locked!\n");
        success = false;
    }
    else
    {
        if (SDL_LockTexture(mTexture, NULL, &mRawPixels, &mRawPitch) != 0)
        {
            SDL_Log("Unable to lock texture! %s\n", SDL_GetError());
            success = false;
        }
    }

    return success;
}

bool SDLTextureWrapper::unlockTexture()
{
    bool success = true;

    if (mRawPixels == NULL)
    {
        SDL_Log("Texture is not locked!\n");
        success = false;
    }
    else
    {
        SDL_UnlockTexture(mTexture);
        mRawPixels = NULL;
        mRawPitch = 0;
    }

    return success;
}

void SDLTextureWrapper::copyRawPixels32(void *pixels)
{
    if (mRawPixels != NULL)
    {
        memcpy(mRawPixels, pixels, mRawPitch * mHeight);
    }
}
