#ifndef __SIMPLESHADER_H__
#define __SIMPLESHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"
#include "IShader.h"
#include "Utility.h"

class SimpleShader: public IShader {
public:
    SimpleShader(Matrix4* m_mvp, Matrix4* s_mvp, Matrix4* m_model, TGAImage* image, Vector3 lightDir, float* s_Buffer);
    ~SimpleShader();

    Vector4 vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir);
    TGAColor fragment(const Vector3& bar);
    Matrix3 varying_uv;
    Matrix4 varying_normal;
    Matrix4 varying_clip;
private:
    SimpleShader(const SimpleShader&);
    TGAImage* tgaImage;
    Matrix4 uniform_M;
    Matrix4 uniform_MIT;
    Matrix4 uniform_Mshadow;
    Vector3 light_dir;
    float* shadowBuffer;
};

#endif //__SIMPLESHADER_H__
