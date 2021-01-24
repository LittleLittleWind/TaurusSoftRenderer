#include "SimpleShader.h"

SimpleShader::SimpleShader(Matrix4* m_mvp, Matrix4* m_model, TGAImage *image, Vector3 lightDir):mvp(m_mvp), model(m_model), tgaImage(image), light_dir(lightDir)
{
	
}

SimpleShader::~SimpleShader()
{
	
}

Vector4 SimpleShader::vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir)
{
	Vector4 clipCoor = (*mvp) * modelPts;
	varying_uv.setColumn(id, uv);
	Matrix4 normalMatrix = model->invertGeneral().transpose();
	varying_normal.setColumn(id, normalMatrix * normalDir);
	return clipCoor;
}

TGAColor SimpleShader::fragment(const Vector3& bar)
{
	Vector3 buv = varying_uv * bar;
	Vector3 bn = varying_normal * bar;
	bn.normalize();
	float intensity = (bn.dot(light_dir)) * 0.5f + 0.5f;
	TGAColor color = tgaImage->get(buv[0] * tgaImage->get_width(), buv[1] * tgaImage->get_height());
	return color * intensity;
}