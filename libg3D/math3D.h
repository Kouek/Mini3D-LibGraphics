#ifndef _MATH3D_H
#define _MATH3D_H

#define PI 3.1415926
#define DEG_TO_RAD PI/180.f

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 矩阵与向量在内存中的布局：
// Matrix 4x4    | Vector 4x1
// +--+--+--+--+ | +-+
// |0 |1 |2 |3 | | |0|
// +--+--+--+--+ | +-+
// |4 |5 |6 |7 | | |1|
// +--+--+--+--+ | +-+
// |8 |9 |10|11| | |2|
// +--+--+--+--+ | +-+
// |12|13|14|15| | |3|
// +--+--+--+--+ | +-+

static void libg3DPrintMat4x4(const float* mat4x4, const char* name)
{
	if (name != NULL)
		printf("%s", name);
	printf("<<<\n");
	printf("%f\t%f\t%f\t%f\n", mat4x4[0], mat4x4[1], mat4x4[2], mat4x4[3]);
	printf("%f\t%f\t%f\t%f\n", mat4x4[4], mat4x4[5], mat4x4[6], mat4x4[7]);
	printf("%f\t%f\t%f\t%f\n", mat4x4[8], mat4x4[9], mat4x4[10], mat4x4[11]);
	printf("%f\t%f\t%f\t%f\n", mat4x4[12], mat4x4[13], mat4x4[14], mat4x4[15]);
	printf(">>>\n");
}

static void libg3DPrintVec4(const float* vec4, const char* name)
{
	if (name != NULL)
		printf("%s", name);
	printf("<<<\n");
	printf("%f\t%f\t%f\t%f\n", vec4[0], vec4[1], vec4[2], vec4[3]);
	printf(">>>\n");
}

#define SET_IDENTITY4X4(mat) \
	memset(##mat, 0, sizeof(float) * 16);\
	##mat[0] = ##mat[5] = ##mat[10] = ##mat[15] = 1.f;

static float* libg3DGenIdentity4x4()
{
	float* mat = NULL;
	mat = malloc(sizeof(float) * 16);
	SET_IDENTITY4X4(mat);
	return mat;
}

static void libg3DSetIdentity4x4(float* mat4x4)
{
	SET_IDENTITY4X4(mat4x4);
}

#undef SET_IDENTITY4X4

// win32的这些宏会导致符号重定义问题，将其取消
#undef near
#undef far

#define SET_PROJECTION4X4(mat) \
	memset(##mat, 0, sizeof(float) * 16);\
	##mat[0] = 2.f * near / (right - left);\
	##mat[2] = (right + left) / (right - left);\
	##mat[5] = 2.f * near / (top - bottom);\
	##mat[7] = (top + bottom) / (top - bottom);\
	##mat[10] = (far + near) / (near - far);\
	##mat[11] = 2.f * near * far / (near - far);\
	##mat[14] = -1.f;

static float* libg3DGenPerspective4x4FromRaw(
	float left, float right,
	float bottom, float top,
	float near, float far)
{
	float* mat = NULL;
	mat = malloc(sizeof(float) * 16);
	SET_PROJECTION4X4(mat);
	return mat;
}

static void libg3DSetPerspective4x4FromRaw(
	float* mat4x4,
	float left, float right,
	float bottom, float top,
	float near, float far)
{
	SET_PROJECTION4X4(mat4x4);
}

#undef SET_PROJECTION4X4

static float* libg3DGenPerspective4x4(float near, float far, float degFOV, float ratioHW)
{
	float top = near * tanf(degFOV / 2 * DEG_TO_RAD);
	float right = top * ratioHW;
	return libg3DGenPerspective4x4FromRaw(-right, right, -top, top, near, far);
}

static void libg3DSetPerspective4x4(
	float* mat4x4,
	float near,
	float far, float degFOV, float ratioHW)
{
	float top = near * tanf(degFOV / 2 * DEG_TO_RAD);
	float right = top * ratioHW;
	libg3DSetPerspective4x4FromRaw(
		mat4x4, -right, right, -top, top, near, far);
}

#define MUL_THEN_ADD(lft, rht, row, col, stride) \
	lft[row * 4] * rht[col] \
	+ lft[row * 4 + 1] * rht[col + stride] \
	+ lft[row * 4 + 2] * rht[col + stride * 2] \
	+ lft[row * 4 + 3] * rht[col + stride * 3]

static void libg3DMat4x4MulVec4(float* outVec4, const float* lftMat4x4, const float* rhtVec4)
{
	// 使用临时变量v4，避免 outVec4==rhtVec4 时的原地运算问题
	float v4[4];
	v4[0] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 0, 0, 1);
	v4[1] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 1, 0, 1);
	v4[2] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 2, 0, 1);
	v4[3] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 3, 0, 1);
	memcpy(outVec4, v4, sizeof(float) * 4);
}

static void libg3DMat4x4MulMat4x4(float* outMat4x4, const float* lftMat4x4, const float* rhtMat4x4)
{
	// 使用临时变量m4，避免 outMat4x4==lft/rhtMar4x4 时的原地运算问题
	float m4[16];
	m4[0] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 0, 4);
	m4[1] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 1, 4);
	m4[2] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 2, 4);
	m4[3] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 3, 4);
	m4[4] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 0, 4);
	m4[5] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 1, 4);
	m4[6] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 2, 4);
	m4[7] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 3, 4);
	m4[8] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 0, 4);
	m4[9] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 1, 4);
	m4[10] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 2, 4);
	m4[11] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 3, 4);
	m4[12] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 0, 4);
	m4[13] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 1, 4);
	m4[14] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 2, 4);
	m4[15] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 3, 4);
	memcpy(outMat4x4, m4, sizeof(float) * 16);
}

#undef MUL_THEN_ADD

static void libg3DMat4x4Scale(float* inOutMat4x4, float sX, float sY, float sZ)
{
	// 等价于左乘 [[sX,0,0,0], [0,sY,0,0], [0,0,sZ,0], [0,0,0,1]]。
	for (unsigned char col = 0; col < 4; ++col)
	{
		unsigned char idx;
		idx = 0 * 4 + col;
		inOutMat4x4[idx] *= sX;
		idx = 1 * 4 + col;
		inOutMat4x4[idx] *= sY;
		idx = 2 * 4 + col;
		inOutMat4x4[idx] *= sZ;
	}
}

static void libg3DMat4x4Translate(float* inOutMat4x4, float tX, float tY, float tZ)
{
	// 等价于左乘 [[1,0,0,tX], [0,1,0,tY], [0,0,1,tZ], [0,0,0,1]]。
	//   由于未经透视投影前，不管如何变换，第3行均为[0,0,0,1]，
	//   可省略第3行的计算。
	inOutMat4x4[3] += tX;
	inOutMat4x4[7] += tY;
	inOutMat4x4[11] += tZ;
}

static void libg3DMat4x4Rotate(float* inOutMat4x4, float degX, float degY, float degZ)
{
	float coss[] = {
		cosf(degX * DEG_TO_RAD),
		cosf(degY * DEG_TO_RAD),
		cosf(degZ * DEG_TO_RAD) };
	float sins[] = {
		sinf(degX * DEG_TO_RAD),
		sinf(degY * DEG_TO_RAD),
		sinf(degZ * DEG_TO_RAD) };
	float right[16] = { 0 };
	float out[16] = { 0 };
	right[0] = coss[1] * coss[2];
	right[1] = -coss[0] * sins[2];
	right[2] = sins[1];
	right[4] = sins[0] * sins[1] * coss[2] + coss[0] * sins[2];
	right[5] = coss[0] * coss[2] - sins[0] * sins[1] * sins[2];
	right[6] = -sins[0] * sins[1];
	right[8] = sins[0] * sins[2] - coss[0] * sins[1] * coss[2];
	right[9] = coss[0] * sins[1] * sins[2] + sins[0] * coss[2];
	right[10] = coss[0] * coss[1];
	right[15] = 1;
	libg3DMat4x4MulMat4x4(out, inOutMat4x4, right);
	memcpy(inOutMat4x4, out, sizeof(float) * 16);
}

static void libg3DNormalizeVec3(float* vec3)
{
	float rLen = sqrtf(vec3[0] * vec3[0] + vec3[1] * vec3[1] + vec3[2] * vec3[2]);
	rLen = 1.f / rLen;
	vec3[0] *= rLen;
	vec3[1] *= rLen;
	vec3[2] *= rLen;
}

static void libg3DVec3CrossVec3(float* outVec3, const float* lftVec3, const float* rhtVec3)
{
	// 使用临时变量v3，避免 outVec3==lft/rhtVec3 时的原地运算问题
	float v3[3];
	v3[0] = lftVec3[1] * rhtVec3[2] - lftVec3[2] * rhtVec3[1];
	v3[1] = lftVec3[2] * rhtVec3[0] - lftVec3[0] * rhtVec3[2];
	v3[2] = lftVec3[0] * rhtVec3[1] - lftVec3[1] * rhtVec3[0];
	memcpy(outVec3, v3, sizeof(float) * 3);
}

static void libg3DMat4x4LookAt(
	float* mat4x4,
	const float* eyeVec3,
	const float* centerVec3)
{
	float up[3] = { 0,1.f,0 };
	float right[3];
	float forward[3] = {
		centerVec3[0] - eyeVec3[0],
		centerVec3[1] - eyeVec3[1],
		centerVec3[2] - eyeVec3[2]
	};
	libg3DVec3CrossVec3(right, forward, up);
	libg3DVec3CrossVec3(up, right, forward);
	libg3DNormalizeVec3(forward);
	libg3DNormalizeVec3(right);
	libg3DNormalizeVec3(up);
	mat4x4[0] = right[0];
	mat4x4[4] = right[1];
	mat4x4[8] = right[2];
	mat4x4[1] = up[0];
	mat4x4[5] = up[1];
	mat4x4[9] = up[2];
	mat4x4[2] = -forward[0];
	mat4x4[6] = -forward[1];
	mat4x4[10] = -forward[2];
	mat4x4[3] = eyeVec3[0];
	mat4x4[7] = eyeVec3[1];
	mat4x4[11] = eyeVec3[2];
	mat4x4[15] = 1.f;
}

static void libg3DMatInverse(float* mat4x4)
{
	// 三维图形库中，一般只对位姿矩阵应用逆变换
	// 可以简化求逆为
	//   新旋转矩阵 = 旧旋转矩阵的转置
	float t[3];
	t[0] = mat4x4[1];
	mat4x4[1] = mat4x4[4];
	mat4x4[4] = t[0];
	t[0] = mat4x4[2];
	mat4x4[2] = mat4x4[8];
	mat4x4[8] = t[0];
	t[0] = mat4x4[6];
	mat4x4[6] = mat4x4[9];
	mat4x4[9] = t[0];
	//   新位移 = 新旋转矩阵 * 旧位移
	t[0] = mat4x4[3];
	t[1] = mat4x4[7];
	t[2] = mat4x4[11];
	mat4x4[3] = -1.f * (mat4x4[0] * t[0] + mat4x4[1] * t[1] + mat4x4[2] * t[2]);
	mat4x4[7] = -1.f * (mat4x4[4] * t[0] + mat4x4[5] * t[1] + mat4x4[6] * t[2]);
	mat4x4[11] = -1.f * (mat4x4[8] * t[0] + mat4x4[9] * t[1] + mat4x4[10] * t[2]);
}

#endif // !_MATH3D_H
