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
/// 使用某个颜色填充视口
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
/// 在顶点着色器中应用mat4x4作为MVP矩阵
/// </summary>
/// <param name="mat4x4Ptr">4x4矩阵</param>
void libg3DApplyMVPMat(const float* const* mat4x4Ptr);

/// <summary>
/// 在片段着色器中应用tex作为纹理
/// </summary>
/// <param name="texPtr">材质结构体指针</param>
void libg3DApplyTexture(const struct Texture* const* texPtr);

/// <summary>
/// 纹理坐标超出[0,1]时的对应方式
/// </summary>
/// <param name="mode">对应模式</param>
void libg3DSetTextureWarpMode(enum TextureWarpMode warpMode);

/// <summary>
/// 片段面积大于等于纹理体素面积时，采用缩小过滤
/// </summary>
/// <param name="mode">过滤模式</param>
void libg3DSetTextureMinFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// 片段面积小于纹理体素面积时，采用放大过滤
/// </summary>
/// <param name="mode">过滤模式</param>
void libg3DSetTextureMagFilterMode(enum TextureFilterMode filtMode);

/// <summary>
/// 指定顶点属性中各属性所占长度（以sizeof(float)为单位）
/// </summary>
void libg3DSetVertAttributes(unsigned int posNum, unsigned int colorNum, unsigned int texCoordNum);

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
