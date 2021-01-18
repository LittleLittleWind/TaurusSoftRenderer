#include "PhongShader.h"

PhongShader::PhongShader(Matrix4* m, TGAImage *image):mvp(m), tgaImage(image)
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
	Vector3 bn = varying_uv * bar;
	return tgaImage->get(bn[0] * tgaImage->get_width(), bn[1] * tgaImage->get_height());
}