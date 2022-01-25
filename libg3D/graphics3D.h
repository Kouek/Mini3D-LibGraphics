#ifndef _GRAPHICS3D_H
#define _GRAPHICS3D_H

#include "math3D.h"
#include "texture.h"

// �ײ�ͼ�����
enum LIBG3D_PRIMITIVE
{
	LIBG3D_POINTS,
	LIBG3D_LINES,
	LIBG3D_TRIANGLES
};

/// <summary>
/// ��ȡ���ڿ�ȣ�������Ϊ��λ��������LibGraphics�Ǹ���ֵ�double������ֵ
/// </summary>
/// <returns>���ڿ�ȣ�������Ϊ��λ��</returns>
int libg3DGetWindowPixelWidth();

/// <summary>
/// ��ȡ���ڸ߶ȣ�������Ϊ��λ��������LibGraphics�Ǹ���ֵ�double������ֵ
/// </summary>
/// <returns>���ڸ߶ȣ�������Ϊ��λ��</returns>
int libg3DGetWindowPixelHeight();

/// <summary>
/// ���õ�ǰ�ӿڳߴ�
/// </summary>
/// <param name="x">���봰�����µĺ�����</param>
/// <param name="y">���봰�����µ�������</param>
/// <param name="width">���</param>
/// <param name="height">�߶�</param>
void libg3DViewport(int x, int y, int width, int height);

/// <summary>
/// ʹ��ĳ����ɫ����ӿ�
/// </summary>
/// <param name="r">��</param>
/// <param name="g">��</param>
/// <param name="b">��</param>
void libg3DClearWithColor(float r, float g, float b);

/// <summary>
/// ����ӿڵ���Ȼ���
/// </summary>
void libg3DClearDepthBuffer();

/// <summary>
/// �ڶ�����ɫ����Ӧ��mat4x4��ΪMVP����
/// </summary>
/// <param name="mat4x4Ptr">4x4����</param>
void libg3DApplyMVPMat(const float* const* mat4x4Ptr);

/// <summary>
/// ��Ƭ����ɫ����Ӧ��tex��Ϊ����
/// </summary>
/// <param name="texPtr">���ʽṹ��ָ��</param>
void libg3DApplyTexture(const struct Texture* const* texPtr);

/// <summary>
/// �������곬��[0,1]ʱ�Ķ�Ӧ��ʽ
/// </summary>
/// <param name="mode">��Ӧģʽ</param>
void libg3DSetTextureWarpMode(enum TextureWarpMode warpMode);

/// <summary>
/// Ƭ��������ڵ��������������ʱ��������С����
/// </summary>
/// <param name="mode">����ģʽ</param>
void libg3DSetTextureMinFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// Ƭ�����С�������������ʱ�����÷Ŵ����
/// </summary>
/// <param name="mode">����ģʽ</param>
void libg3DSetTextureMagFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// ָ�����������и�������ռ���ȣ���sizeof(float)Ϊ��λ��
/// </summary>
void libg3DSetVertAttributes(unsigned int posNum, unsigned int colorNum, unsigned int texCoordNum);

/// <summary>
/// ���ƶ�����������verts
/// </summary>
/// <param name="primitive">���Ƶ�ͼԪ����</param>
/// <param name="verts">������������</param>
/// <param name="numOfVerts">�������</param>
void libg3DDrawVerts(enum LIBG3D_PRIMITIVE primitive, const float* verts, unsigned int numOfVerts);

/// <summary>
/// ������������indices���ƶ�����������verts
/// </summary>
/// <param name="primitive">���Ƶ�ͼԪ����</param>
/// <param name="verts">������������</param>
/// <param name="indices">��������</param>
/// <param name="numOfVerts">��������</param>
void libg3DDrawVertsByIndices(
		enum LIBG3D_PRIMITIVE primitive,
		const float* verts,
		const unsigned int* indices,
		unsigned int numOfIndices
	);

#endif // !_GRAPHICS3D_H
