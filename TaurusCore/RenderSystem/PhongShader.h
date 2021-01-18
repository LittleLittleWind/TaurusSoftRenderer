#ifndef __PHONGSHADER_H__
#define __PHONGSHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"

class PhongShader {
public:
    PhongShader(Matrix4* m, TGAImage* image, Vector3 lightDir);
    ~PhongShader();

    Vector4 vertex(const Vector3& modelCoor);
    TGAColor fragment(const Vector3& bar);
    Matrix3 varying_uv;
    Matrix3 varying_normal;
    Vector3 light_dir;
private:
    PhongShader(const PhongShader&);
    TGAImage* tgaImage;
    Matrix4* mvp;
};

#endif //__PHONGSHADER_H__
