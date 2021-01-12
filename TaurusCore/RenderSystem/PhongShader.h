#ifndef __PHONGSHADER_H__
#define __PHONGSHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"

class PhongShader {
public:
    PhongShader(TGAImage* image);
    ~PhongShader();

    Vector3 vertex(Vector3 modelCoor);
    TGAColor fragment(const Vector3 bar);
    Matrix3 varying_uv;
private:
    PhongShader(const PhongShader&);
    TGAImage* tgaImage;
};

#endif //__PHONGSHADER_H__
