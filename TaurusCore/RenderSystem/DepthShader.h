#ifndef __DEPTHSHADER_H__
#define __DEPTHSHADER_H__

#include <stdio.h>
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"
#include "tgaimage.h"
#include "IShader.h"

class DepthShader: public IShader {
public:
    DepthShader(Matrix4* m_mvp);
    ~DepthShader();

    Vector4 vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir);
    TGAColor fragment(const Vector3& bar);
    Matrix3 varying_uv;
    Matrix4 varying_normal;
private:
    DepthShader(const DepthShader&);
    Matrix4 uniform_M;
    Matrix4 clipCoors;
};

#endif //__DEPTHSHADER_H__
