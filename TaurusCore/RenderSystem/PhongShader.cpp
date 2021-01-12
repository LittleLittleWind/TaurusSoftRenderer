#include "PhongShader.h"

PhongShader::PhongShader(TGAImage *image):tgaImage(image)
{
	
}

PhongShader::~PhongShader()
{
	
}

Vector3 PhongShader::vertex(Vector3 modelCoor)
{
	int x0 = modelCoor.x * 8 + 640 / 2.;
	int y0 = modelCoor.y * 8 + 480 / 2;
	float z0 = modelCoor.z;
	return Vector3(x0, y0, z0);
}

TGAColor PhongShader::fragment(const Vector3 bar)
{
	Vector3 bn = varying_uv * bar;
	return tgaImage->get(bn[0] * tgaImage->get_width(), bn[1] * tgaImage->get_height());
	
}