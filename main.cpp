#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <fstream>
using namespace std;
const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int TILE_WIDTH = 80;
const int TILE_HEIGHT = 80;

const int TOTAL_TILES = 192;
enum TILE
{
    TILE_RED,
    TILE_GREEN,
    TILE_BLUE,
    TILE_CENTER,
    TILE_TOP,
    TILE_TOPRIGHT,
    TILE_RIGHT,
    TILE_BOTTOMRIGHT,
    TILE_BOTTOM,
    TILE_BOTTMLEFT,
    TILE_LEFT,
    TILE_TOPLEFT,
    TOTAL_TILE_SPRITES
};
const string WINDOW_TITLE = "sigma";

void logSDLError(ostream &os, const string &msg, bool fatal = false);
void logIMGError(ostream &os, const string &msg, bool fatal = false);
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer);
void quitSDL(SDL_Window* window, SDL_Renderer* renderer);
void waitUntilKeyPress();
bool checkCollision(SDL_Rect a, SDL_Rect b)
{
    int topA, bottomA, leftA, rightA;
    int topB, bottomB, leftB, rightB;
    topA = a.y;
    bottomA = a.y + a.h;
    leftA = a.x;
    rightA = a.x + a.w;
    topB = b.y;
    bottomB = b.y + b.h;
    leftB = b.x;
    rightB = b.x + b.w;
    if(topA >= bottomB || topB >= bottomA || leftA >= rightB || leftB >= rightA)return false;
    return true;
}
bool checkCollisionTile(SDL_Rect a, Tile &tiles[])
{
    for(int i=0;i<TOTAL_TILES;i++)
    {
        if(checkCollision(a, tiles[i].getBox()))return true;
    }
    return false;
}
class Tile
{
    SDL_Rect mBox;
    int mType;
public:
    Tile(int x, int y, int tileType);
    void render(SDL_Recr &camera);
    int getType();
    SDL_Rect getBox();
};
class LDot
{
    int mVelX;
    int mVelY;
    int mPosX;
    int mPosY;
    SDL_Rect mCollisionBox;
    SDL_Renderer* mRenderer;
    SDL_Texture* mTexture;
    void shiftCollisionBox();
public:
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;
    static const int DOT_VEL = 10;
    LDot();
    bool loadTextureForDot(string path);
    void handleEvent(SDL_Event* e);
    void Move(Tile *tiles[]);
    void render(SDL_Rect &camera);
    void setRenderer(SDL_Renderer* renderer);
    void setCamera(SDL_Rect &camera)
};

int main(int argc, char* argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer);

    LDot dot;
    dot.setRenderer(renderer);
    if(!dot.loadTextureForDot("dot.bmp"))logIMGError(cout, "Load Image", true);
    bool quit = false;
    SDL_Event e;
    while(!quit)
    {
        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
            {
                quit = true;
            }
            else
            {
                dot.handleEvent(&e);
            }
        }
        dot.Move();
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);
        dot.render();
        SDL_RenderPresent(renderer);
    }

    waitUntilKeyPress();
    quitSDL(window, renderer);
}

void logSDLError(ostream &os, const string &msg, bool fatal)
{
    os << msg << " Error: " << SDL_GetError() << '\n';
    if(fatal)
    {
        SDL_Quit();
        exit(1);
    }
}
void logIMGError(ostream &os, const string &msg, bool fatal)
{
    os << msg << " Error: " << IMG_GetError() << '\n';
    if(fatal)
    {
        SDL_Quit();
        exit(1);
    }
}
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)logSDLError(cout, "InitSDL", true);
    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == nullptr)logSDLError(cout, "CreateWindow", true);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )logIMGError(cout, "InitPNG", true);
    imgFlags = IMG_INIT_JPG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )logIMGError(cout, "InitJPG", true);
}
void quitSDL(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();
}
void waitUntilKeyPress()
{
    SDL_Event e;
    while(true)
    {
        if(SDL_WaitEvent(&e) != 0 && (e.type == SDL_QUIT || e.type == SDL_KEYDOWN))
        {
            return;
        }
        SDL_Delay(100);
    }
}
LDot::LDot()
{
    mVelX = 0;
    mVelY = 0;
    mPosX = 0;
    mPosY = 0;
    mCollisionBox.x = mPosX;
    mCollisionBox.y = mPosY;
    mCollisionBox.w = DOT_WIDTH;
    mCollisionBox.h = DOT_HEIGHT;
    mTexture = nullptr;
    mRenderer = nullptr;
}
bool LDot::loadTextureForDot(string path)
{
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* surface = IMG_Load(path.c_str());
    if(surface == nullptr)logIMGError(cout, "Load Surface", true);
    else
    {
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0xFF, 0xFF));
        newTexture = SDL_CreateTextureFromSurface(mRenderer, surface);
        if(newTexture == nullptr)logSDLError(cout, "Load Texture", true);
    }
    mTexture = newTexture;
    return (mTexture != nullptr);
}
void LDot::setRenderer(SDL_Renderer* renderer)
{
    mRenderer = renderer;
}
void LDot::handleEvent(SDL_Event* e)
{
    if(e->type == SDL_KEYDOWN && (e->key).repeat == 0)
    {
        switch((e->key).keysym.sym)
        {
        case SDLK_UP:
            mVelY -= DOT_VEL;
            break;
        case SDLK_DOWN:
            mVelY += DOT_VEL;
            break;
        case SDLK_LEFT:
            mVelX -= DOT_VEL;
            break;
        case SDLK_RIGHT:
            mVelX += DOT_VEL;
            break;
        }
    }
    else
    {
        if(e->type == SDL_KEYUP && (e->key).repeat == 0)
        {
            switch((e->key).keysym.sym)
            {
            case SDLK_UP:
                mVelY += DOT_VEL;
                break;
            case SDLK_DOWN:
                mVelY -= DOT_VEL;
                break;
            case SDLK_LEFT:
                mVelX += DOT_VEL;
                break;
            case SDLK_RIGHT:
                mVelX -= DOT_VEL;
                break;
            }
        }
    }
}
void LDot::Move(Tile &tiles[])
{
    mPosX += mVelX;
    shiftCollisionBox();
    if(mPosX < 0 || (mPosX + DOT_WIDTH > SCREEN_WIDTH) || checkCollisionTile(mCollisionBox, tiles))
    {
        mPosX -= mVelX;
        shiftCollisionBox();
    }

    mPosY += mVelY;
    shiftCollisionBox();
    if(mPosY < 0 || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT) || checkCollisionTile(mCollisionBox, tiles))
    {
        mPosY -= mVelY;
        shiftCollisionBox();
    }
}
void LDot::render(SDL_Rect &camera)
{
    SDL_Rect rect = {mPosX - camera.x, mPosY - camera.y, DOT_WIDTH, DOT_HEIGHT};
    SDL_RenderCopy(mRenderer, mTexture, NULL, &rect);
}
void LDot::setCamera(SDL_Rect &camera)
{
    camera.x = mPosX + DOT_WIDTH/2 - SCREEN_WIDTH/2;
    camera.y = mPosY + DOT_HEIGHT/2 - SCREEN_HEIGHT/2;
    if(camera.x < 0)
    {
        camera.x = 0;
    }
    if(camera.y < 0)camera.y = 0;
    if(camera.x + SCREEN_WIDTH > LEVEL_WIDTH)
    {
        camera.x = LEVEL_WIDTH - SCREEN_WIDTH;
    }
    if(camera.y + SCREEN_HEIGHT > LEVEL_WIDTH)
    {
        camera.y = LEVEL_HEIGHT - SCREEN_HEIGHT;
    }
}
void LDot::shiftCollisionBox()
{
    mCollisionBox.x = mPosX;
    mCollisionBox.y = mPosY;
}
Tile::Tile(int x,int y,int tileType)
{
    mBox.x = x;
    mBox.y = y;
    mBox.w = TILE_WIDTH;
    mBox.h = TILE_HEIGHT;
    mType = tileType;
}
void Tile::render(SDL_Rect &camera)
{
    if(checkCollision(mBox, camera))
    {
        SDL_RenderCopy(aRenderer, aTexture, )
    }
}
