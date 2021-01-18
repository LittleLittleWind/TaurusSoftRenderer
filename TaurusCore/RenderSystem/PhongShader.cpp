#include "PhongShader.h"

PhongShader::PhongShader(Matrix4* m, TGAImage *image, Vector3 lightDir):mvp(m), tgaImage(image), light_dir(lightDir)
{
	
}

PhongShader::~PhongShader()
{
	
}

Vector4 PhongShader::vertex(const Vector3& modelCoor)
{
	Vector4 clipCoor = (*mvp) * Vector4(modelCoor.x, modelCoor.y, modelCoor.z, 1);
	return clipCoor;
}

TGAColor PhongShader::fragment(const Vector3& bar)
{
	Vector3 buv = varying_uv * bar;
	Vector3 bn = varying_normal * bar;
	bn.normalize();
	float intensity = (bn.dot(light_dir)) * 0.5f + 0.5f;
	TGAColor color = tgaImage->get(buv[0] * tgaImage->get_width(), buv[1] * tgaImage->get_height());
	return color * intensity;
}