#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include "..\Matrix\Vectors.h"
#include "tgaimage.h"
#include "SimpleShader.h"
#include "DepthShader.h"
#include "Utility.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool UpdateFrame();
private:
    Renderer(const Renderer&);
    void Init();
    void Close();
    SDL_Window* mWindow = NULL;
    SDL_Renderer* mRenderer = NULL;
    int mWidth, mHeight;

    void SDLDrawPixel(int x, int y);
    void line(int x0, int y0, int x1, int y1);
    Vector3 barycentric(Vector3* pts, Vector3 P);
    void triangle(Vector4* pts, IShader* shader, float* targetZBuffer);
    void ShowObjShaded();
    void ShowTextTip();
    float* zBuffer;
    float* shadowBuffer;
    TGAImage tgaImage = TGAImage();
    TGAImage textTipImage = TGAImage();
    SDL_Event e;
    bool isInited;
    bool useWireFrame;
    Vector4 cameraPos;
    float modelYawAngle;
};

#endif //__RENDERER_H__
