#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include "..\Matrix\Vectors.h"
#include "tgaimage.h"
#include "PhongShader.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool UpdateFrame();
private:
    Renderer(const Renderer&);
    void Init(int screenWidth, int ScreenHeight);
    void Close();
    SDL_Window* mWindow = NULL;
    SDL_Renderer* mRenderer = NULL;
    int mWidth, mHeight;

    void SDLDrawPixel(int x, int y);
    void line(int x0, int y0, int x1, int y1);
    Vector3 barycentric(Vector3* pts, Vector3 P);
    void triangle(Vector3* pts, float c, PhongShader *shader);
    void ShowObjShaded();
    void ShowTextTip();
    int* zbuffer;
    TGAImage tgaImage = TGAImage();
    TGAImage textTipImage = TGAImage();
    SDL_Event e;
    bool isInited;
    bool useWireFrame;
};

#endif //__RENDERER_H__
