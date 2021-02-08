#include "DepthShader.h"

DepthShader::DepthShader(Matrix4* m_mvp) :uniform_M(*m_mvp)
{
	depthPass = true;
}

DepthShader::~DepthShader()
{

}

Vector4 DepthShader::vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir)
{
	Vector4 clipCoor = uniform_M * modelPts;
	clipCoors.setColumn(id, clipCoor);
	return clipCoor;
}

TGAColor DepthShader::fragment(const Vector3& bar)
{
	TGAColor color;
	return color;
}