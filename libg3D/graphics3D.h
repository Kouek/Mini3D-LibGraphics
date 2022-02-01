#ifndef _GRAPHICS3D_H
#define _GRAPHICS3D_H

#include "math3D.h"
#include "texture.h"

// 底层图形组件
enum LIBG3D_PRIMITIVE
{
	LIBG3D_POINTS,
	LIBG3D_LINES,
	LIBG3D_TRIANGLES
};

// 面剔除模式
enum LIBG3D_CULL_FACE_MODE
{
	LIBG3D_CULL_BACK,
	LIBG3D_CULL_FRONT,
	LIBG3D_CULL_NO_FACE
};

/// <summary>
/// 获取窗口宽度，以像素为单位，而不是LibGraphics那个奇怪的double类型数值
/// </summary>
/// <returns>窗口宽度（以像素为单位）</returns>
int libg3DGetWindowPixelWidth();

/// <summary>
/// 获取窗口高度，以像素为单位，而不是LibGraphics那个奇怪的double类型数值
/// </summary>
/// <returns>窗口高度（以像素为单位）</returns>
int libg3DGetWindowPixelHeight();

/// <summary>
/// 设置当前视口尺寸
/// </summary>
/// <param name="x">距离窗口左下的横坐标</param>
/// <param name="y">距离窗口左下的纵坐标</param>
/// <param name="width">宽度</param>
/// <param name="height">高度</param>
void libg3DViewport(int x, int y, int width, int height);

/// <summary>
/// 使用某个颜色填充视口，
/// rgb各值需在[0,1]内
/// </summary>
/// <param name="r">红</param>
/// <param name="g">绿</param>
/// <param name="b">蓝</param>
void libg3DClearWithColor(float r, float g, float b);

/// <summary>
/// 清除视口的深度缓冲
/// </summary>
void libg3DClearDepthBuffer();

/// <summary>
/// 在顶点着色器中应用*mat4x4Ptr作为MVP矩阵，
/// 默认为“NULL”，
/// mat4x4Ptr及*mat4x4Ptr不得为NULL
/// </summary>
/// <param name="mat4x4Ptr">4x4矩阵指针</param>
void libg3DApplyMVPMat(const float* const mat4x4Ptr[16]);

/// <summary>
/// 在片段着色器中应用*texPtr作为纹理，
/// 默认为“NULL”，
/// texPtr为NULL时使用绘制颜色，不绘制纹理
/// </summary>
/// <param name="texPtr">材质结构体指针</param>
void libg3DApplyTexture(const struct Texture* const* texPtr);

/// <summary>
/// 纹理坐标超出[0,1]时的对应方式，
/// 默认为“止步边缘”
/// </summary>
/// <param name="mode">对应模式</param>
void libg3DSetTextureWarpMode(enum TextureWarpMode warpMode);

/// <summary>
/// 设置片段面积大于等于纹理体素面积时，采用的缩小过滤模式，
/// 默认为“（双）线性过滤”
/// </summary>
/// <param name="mode">过滤模式</param>
void libg3DSetTextureMinFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// 设置片段面积小于纹理体素面积时，采用的放大过滤模式，
/// 默认为“最近邻过滤”
/// </summary>
/// <param name="mode">过滤模式</param>
void libg3DSetTextureMagFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// 指定顶点属性中各属性所占长度（以sizeof(float)为单位）
/// </summary>
void libg3DSetVertAttributes(unsigned int posNum, unsigned int colorNum, unsigned int texCoordNum);

/// <summary>
/// 设置面剔除模式，默认为“剔除背面”
/// </summary>
/// <param name="cullMode">剔除模式</param>
void libg3DSetCullFaceMode(enum LIBG3D_CULL_FACE_MODE cullMode);

/// <summary>
/// 绘制顶点属性数组verts
/// </summary>
/// <param name="primitive">绘制的图元类型</param>
/// <param name="verts">顶点属性数组</param>
/// <param name="numOfVerts">顶点个数</param>
void libg3DDrawVerts(enum LIBG3D_PRIMITIVE primitive, const float* verts, unsigned int numOfVerts);

/// <summary>
/// 依据索引数组indices绘制顶点属性数组verts
/// </summary>
/// <param name="primitive">绘制的图元类型</param>
/// <param name="verts">顶点属性数组</param>
/// <param name="indices">索引数组</param>
/// <param name="numOfVerts">索引个数</param>
void libg3DDrawVertsByIndices(
		enum LIBG3D_PRIMITIVE primitive,
		const float* verts,
		const unsigned int* indices,
		unsigned int numOfIndices
	);

#endif // !_GRAPHICS3D_H
