#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include "..\geometry.h"
#include "tgaimage.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool UpdateFrame();
private:
    Renderer(const Renderer&);
    Renderer& operator = (const Renderer&);
    void Init(int screenWidth, int ScreenHeight);
    void Close();
    SDL_Window* mWindow = NULL;
    SDL_Renderer* mRenderer = NULL;
    int mWidth, mHeight;

    void SDLDrawPixel(int x, int y);
    void line(int x0, int y0, int x1, int y1);
    Vec3f barycentric(Vec3f* pts, Vec3f P);
    void triangle(Vec3f* pts, Vec2f* uvs, float c);
    void ShowObjWireframe();
    void ShowObjShaded();
    int* zbuffer;
    TGAImage tgaImage = TGAImage();
    SDL_Event e;
    bool isInited;
};

#endif //__RENDERER_H__
