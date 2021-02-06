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

TGAColor SimpleShader::fragment(const Vector3& bar, const int screenWiddth, const int screenHeight)
{
	Vector3 buv = varying_uv * bar;
	Vector3 bn = varying_normal * bar;
	bn.normalize();
	float intensity = (bn.dot(light_dir)) * 0.5f + 0.5f;
	TGAColor color = tgaImage->get(buv[0] * tgaImage->get_width(), buv[1] * tgaImage->get_height());

	Matrix4 transformedPos = shadowTransform * varying_clip;
	Vector4 v1 = transformedPos.getColumn(0);
	Vector4 v2 = transformedPos.getColumn(1);
	Vector4 v3 = transformedPos.getColumn(2);
	v1 /= v1.w;
	v2 /= v2.w;
	v3 /= v3.w;
	v1.x = (v1.x / 2 + 0.5) * screenWiddth;
	v1.y = (v1.y / 2 + 0.5) * screenHeight;
	v1.z = (v1.z / 2 + 0.5);
	v2.x = (v2.x / 2 + 0.5) * screenWiddth;
	v2.y = (v2.y / 2 + 0.5) * screenHeight;
	v2.z = (v2.z / 2 + 0.5);
	v3.x = (v3.x / 2 + 0.5) * screenWiddth;
	v3.y = (v3.y / 2 + 0.5) * screenHeight;
	v3.z = (v3.z / 2 + 0.5);
	Vector4 clipPos = v1 * bar.x + v2 * bar.y + v3 * bar.z;
	
	bool notInShadow = false;
	if (clipPos.x<0 || clipPos.x>screenWiddth || clipPos.y<0 || clipPos.y>screenHeight || clipPos.z < 0 || clipPos.z>1)
		notInShadow = true;
	else
	{
		int idx = int(clipPos[0] + clipPos[1] * screenWiddth);
		notInShadow = shadowBuffer[idx] > clipPos[2] - 0.1;
	}
	float shadow = .3 + .7 * notInShadow;
	return color * intensity;
}

