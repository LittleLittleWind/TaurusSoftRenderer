#include "SimpleShader.h"

SimpleShader::SimpleShader(Matrix4* m_mvp, Matrix4* s_mvp, Matrix4* m_model, TGAImage *image, Vector3 lightDir, float* s_buffer):mvp(m_mvp), model(m_model), tgaImage(image), light_dir(lightDir), shadowBuffer(s_buffer)
{
	light_dir.normalize();
	shadowTransform = (*s_mvp);
}

SimpleShader::~SimpleShader()
{
	depthPass = false;
}

Vector4 SimpleShader::vertex(int id, const Vector4& modelPts, const Vector3& uv, const Vector4& normalDir)
{
	Vector4 clipCoor = (*mvp) * modelPts;
	varying_uv.setColumn(id, uv);
	Matrix4 normalMatrix = model->invertGeneral().transpose();
	varying_normal.setColumn(id, normalMatrix * normalDir);
	varying_clip.setColumn(id, modelPts);
	return clipCoor;
}

TGAColor SimpleShader::fragment(const Vector3& bar)
{
	Vector3 buv = varying_uv * bar;
	Vector3 bn = varying_normal * bar;
	bn.normalize();
	float intensity = (bn.dot(light_dir)) * 0.5f + 0.5f;
	TGAColor color = tgaImage->get(buv[0] * tgaImage->get_width(), buv[1] * tgaImage->get_height());

	Vector3 clipPos = varying_clip * bar;
	Vector4 transformedPos = shadowTransform * Vector4(clipPos.x, clipPos.y, clipPos.z, 1);
	transformedPos /= transformedPos.w;
	transformedPos.x = (transformedPos.x / 2 + 0.5) * SCREEN_WIDTH;
	transformedPos.y = (transformedPos.y / 2 + 0.5) * SCREEN_HEIGHT;
	transformedPos.z = (transformedPos.z / 2 + 0.5);
	bool notInShadow = false;
	if (transformedPos.x < 0 || transformedPos.x > SCREEN_WIDTH || transformedPos.y < 0 || transformedPos.y > SCREEN_HEIGHT || transformedPos.z < 0 || transformedPos.z > 1)
		notInShadow = true;
	else
	{
		int idx = (int)transformedPos.x + (int)transformedPos.y * SCREEN_WIDTH;
		notInShadow = shadowBuffer[idx] > transformedPos.z - 0.008;
	}
	float shadow = .2 + .8 * notInShadow;
	return color * intensity * shadow;
}

