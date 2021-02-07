#ifndef __UTILITY_H__
#define __UTILITY_H__
#include "..\Matrix\Vectors.h"
#include "..\Matrix\Matrices.h"

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

extern Matrix4 GetModelMatrix(float yawAngle);
extern Matrix4 GetViewMatrix(Vector4 cameraPos, float yawAngle);
extern Matrix4 GetProjectionMatrix(float aspect);

#endif //__UTILITY_H__