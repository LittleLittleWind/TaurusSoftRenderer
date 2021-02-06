#ifndef __UTILITY_H__
#define __UTILITY_H__
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"

Matrix4 GetModelMatrix(float yawAngle);
Matrix4 GetViewMatrix(Vector4 cameraPos, float yawAngle);
Matrix4 GetProjectionMatrix(float aspect);
float clamp(float n, float lower, float upper);

#endif //__UTILITY_H__