#ifndef __PHONGSHADER_H__
#define __PHONGSHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"

class SimpleShader {
public:
    SimpleShader(Matrix4* m_mvp, Matrix4* m_model, TGAImage* image, Vector3 lightDir);
    ~SimpleShader();

    Vector4 vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir);
    TGAColor fragment(const Vector3& bar);
    Matrix3 varying_uv;
    Matrix4 varying_normal;
private:
    SimpleShader(const SimpleShader&);
    TGAImage* tgaImage;
    Matrix4* mvp;
    Matrix4* model;
    Vector3 light_dir;
};

#endif //__PHONGSHADER_H__
