#include "DepthShader.h"

DepthShader::DepthShader(Matrix4* m_mvp) :mvp(m_mvp)
{
	depthPass = true;
}

DepthShader::~DepthShader()
{

}

Vector4 DepthShader::vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir)
{
	Vector4 clipCoor = (*mvp) * modelPts;
	clipCoors.setColumn(id, clipCoor);
	return clipCoor;
}

TGAColor DepthShader::fragment(const Vector3& bar, const int screenWiddth, const int screenHeight)
{
	Vector4 c1 = clipCoors.getColumn(0);
	Vector4 c2 = clipCoors.getColumn(1);
	Vector4 c3 = clipCoors.getColumn(2);
	c1 /= c1.w;
	c2 /= c2.w;
	c3 /= c3.w;
	float dep = (c1.z / 2 + 0.5) * bar.x + (c2.z / 2 + 0.5) * bar.y + (c3.z / 2 + 0.5) * bar.z;
	TGAColor color = TGAColor(dep * 255, dep * 255, dep * 255);
	return color;
}