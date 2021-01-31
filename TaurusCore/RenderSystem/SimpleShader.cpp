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
	//Vector4 clipPos = varying_clip.getColumn(0) * bar.x + varying_clip.getColumn(1) * bar.y + varying_clip.getColumn(2) * bar.z;
	//Vector4 sb_p = shadowTransform * Vector4(clipPos.x, clipPos.y, clipPos.z, clipPos.w);
	//sb_p /= sb_p.w;
	//sb_p.x = (sb_p.x / 2 + 0.5) * screenWiddth;
	//sb_p.y = (sb_p.y / 2 + 0.5) * screenHeight;
	//sb_p.z = (sb_p.z / 2 + 0.5);
	//int idx = int(sb_p[0] + sb_p[1] * screenWiddth);
	//float shadow = .3 + .7 * (shadowBuffer[idx] < sb_p[2]);
	return color * intensity;
}