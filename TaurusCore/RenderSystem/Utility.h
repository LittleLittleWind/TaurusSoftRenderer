#ifndef __UTILITY_H__
#define __UTILITY_H__
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"

Matrix4 GetModelMatrix(float yawAngle);
Matrix4 GetViewMatrix(Vector4 cameraPos);
Matrix4 GetProjectionMatrix(float aspect);

#endif //__UTILITY_H__