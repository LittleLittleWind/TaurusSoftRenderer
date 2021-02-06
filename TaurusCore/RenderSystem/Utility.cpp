#include "Utility.h"


Matrix4 GetModelMatrix(float yawAngle)
{
	float yawRadians = yawAngle * 3.14 / 180;
	Matrix4 scaleM = Matrix4();
	scaleM.setColumn(0, Vector4(1, 0, 0, 0));
	scaleM.setColumn(1, Vector4(0, 1, 0, 0));
	scaleM.setColumn(2, Vector4(0, 0, 1, 0));
	scaleM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 rotationM = Matrix4();
	rotationM.setColumn(0, Vector4(cos(yawRadians), 0, -sin(yawRadians), 0));
	rotationM.setColumn(1, Vector4(0, 1, 0, 0));
	rotationM.setColumn(2, Vector4(sin(yawRadians), 0, cos(yawRadians), 0));
	rotationM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 transformM = Matrix4();
	transformM.setColumn(0, Vector4(1, 0, 0, 0));
	transformM.setColumn(1, Vector4(0, 1, 0, 0));
	transformM.setColumn(2, Vector4(0, 0, 1, 0));
	transformM.setColumn(3, Vector4(0, 0, 0, 1));
	return transformM * rotationM * scaleM;
}


Matrix4 GetViewMatrix(Vector4 cameraPos, float yawAngle)
{
	float yawRadians = yawAngle * 3.14 / 180;
	Matrix4 minusM = Matrix4();
	minusM.setColumn(0, Vector4(1, 0, 0, 0));
	minusM.setColumn(1, Vector4(0, 1, 0, 0));
	minusM.setColumn(2, Vector4(0, 0, -1, 0));
	minusM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 cameraRotationM = Matrix4();
	cameraRotationM.setColumn(0, Vector4(cos(yawRadians), 0, -sin(yawRadians), 0));
	cameraRotationM.setColumn(1, Vector4(0, 1, 0, 0));
	cameraRotationM.setColumn(2, Vector4(sin(yawRadians), 0, cos(yawRadians), 0));
	cameraRotationM.setColumn(3, Vector4(0, 0, 0, 1));
	Matrix4 cameraTransformM = Matrix4();
	cameraTransformM.setColumn(0, Vector4(1, 0, 0, 0));
	cameraTransformM.setColumn(1, Vector4(0, 1, 0, 0));
	cameraTransformM.setColumn(2, Vector4(0, 0, 1, 0));
	cameraTransformM.setColumn(3, cameraPos);
	return minusM * cameraRotationM.invert() * cameraTransformM.invert();
}

Matrix4 GetProjectionMatrix(float aspect)
{
	float fovAngle = 60 * 3.14 / 180;
	float f = 1000, n = 0.3;
	Matrix4 projectionMatrix = Matrix4();
	projectionMatrix.setColumn(0, Vector4(1 / std::tan(fovAngle / 2) / aspect, 0, 0, 0));
	projectionMatrix.setColumn(1, Vector4(0, 1 / std::tan(fovAngle / 2), 0, 0));
	projectionMatrix.setColumn(2, Vector4(0, 0, -(f + n) / (f - n), -1));
	projectionMatrix.setColumn(3, Vector4(0, 0, -2 * f * n / (f - n), 0));
	return projectionMatrix;
}

float clamp(float n, float lower, float upper)
{
	return std::max(lower, std::min(n, upper));
}
