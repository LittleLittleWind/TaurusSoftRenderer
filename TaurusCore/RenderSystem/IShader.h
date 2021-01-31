#ifndef __ISHADER_H__
#define __ISHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"

class IShader {
public:
    virtual ~IShader() {};

    virtual TGAColor fragment(const Vector3& bar, const int screenWiddth, const int screenHeight) = 0;
    bool depthPass;
};

#endif //__ISHADER_H__