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

// ���޳�ģʽ
enum LIBG3D_CULL_FACE_MODE
{
	LIBG3D_CULL_BACK,
	LIBG3D_CULL_FRONT,
	LIBG3D_CULL_NO_FACE
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
/// ʹ��ĳ����ɫ����ӿڣ�
/// rgb��ֵ����[0,1]��
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
/// �ڶ�����ɫ����Ӧ��*mat4x4Ptr��ΪMVP����
/// Ĭ��Ϊ��NULL����
/// mat4x4Ptr��*mat4x4Ptr����ΪNULL
/// </summary>
/// <param name="mat4x4Ptr">4x4����ָ��</param>
void libg3DApplyMVPMat(const float* const mat4x4Ptr[16]);

/// <summary>
/// ��Ƭ����ɫ����Ӧ��*texPtr��Ϊ����
/// Ĭ��Ϊ��NULL����
/// texPtrΪNULLʱʹ�û�����ɫ������������
/// </summary>
/// <param name="texPtr">���ʽṹ��ָ��</param>
void libg3DApplyTexture(const struct Texture* const* texPtr);

/// <summary>
/// �������곬��[0,1]ʱ�Ķ�Ӧ��ʽ��
/// Ĭ��Ϊ��ֹ����Ե��
/// </summary>
/// <param name="mode">��Ӧģʽ</param>
void libg3DSetTextureWarpMode(enum TextureWarpMode warpMode);

/// <summary>
/// ����Ƭ��������ڵ��������������ʱ�����õ���С����ģʽ��
/// Ĭ��Ϊ����˫�����Թ��ˡ�
/// </summary>
/// <param name="mode">����ģʽ</param>
void libg3DSetTextureMinFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// ����Ƭ�����С�������������ʱ�����õķŴ����ģʽ��
/// Ĭ��Ϊ������ڹ��ˡ�
/// </summary>
/// <param name="mode">����ģʽ</param>
void libg3DSetTextureMagFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// ָ�����������и�������ռ���ȣ���sizeof(float)Ϊ��λ��
/// </summary>
void libg3DSetVertAttributes(unsigned int posNum, unsigned int colorNum, unsigned int texCoordNum);

/// <summary>
/// �������޳�ģʽ��Ĭ��Ϊ���޳����桱
/// </summary>
/// <param name="cullMode">�޳�ģʽ</param>
void libg3DSetCullFaceMode(enum LIBG3D_CULL_FACE_MODE cullMode);

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
