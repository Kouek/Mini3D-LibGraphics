#include "graphics3D.h"

#include "varArray.h"

#include "lib3DExport.h"
#include "../libgraphics/win32Export.h"

#include <math.h>

// WIN32元素
static HBITMAP oldBitmap = NULL, bitmap = NULL;

// 帧缓冲结构
//   一个窗口就由一个帧缓冲操控。
struct FrameBuffer
{
	int windowW, windowH; // 窗口大小
	int viewportX, viewportY; // 视口距离窗口左下角的偏移
	int viewportW, viewportH; // 视口大小
	float viewportArea; // 视口面积
	unsigned char* colorAttachment; // 帧缓冲的颜色缓冲
	float* depthAttachment; // 帧缓冲的深度缓冲
};

static struct FrameBuffer frameBuffer = { 0 };

int libg3DGetWindowPixelWidth()
{
	return GetLibgWindowPixelWidth();
}
int libg3DGetWindowPixelHeight()
{
	return GetLibgWindowPixelHeight();
}

static void markViewportNeedUpdate()
{
	// 将整个viewport标记为需要更新（不是需要擦除）
	// 注意：viewportRECT使用Win32坐标系，
	//   y轴反向，同时原点在窗口左上角
	RECT viewportRECT = {
		frameBuffer.viewportX, // 左
		frameBuffer.windowH - frameBuffer.viewportY - frameBuffer.viewportH, // 上
		frameBuffer.viewportX + frameBuffer.viewportW, // 右
		frameBuffer.windowH - frameBuffer.viewportY // 下
	};
	InvalidateRect(GetLibgWindow(), &viewportRECT, FALSE);
}

void libg3DViewport(int x, int y, int width, int height)
{
	// 保证初始化只进行一次
	if (frameBuffer.colorAttachment == NULL)
	{
		// 获取graphics.c创建的窗口尺寸
		frameBuffer.windowW = GetLibgWindowPixelWidth();
		frameBuffer.windowH = GetLibgWindowPixelHeight();
		// 新建一个与osdc适配的bitmap对象，
		//   BITMAPINFO 详见
		//   https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
		BITMAPINFO bitmapInfo = { {
				sizeof(BITMAPINFOHEADER),
				frameBuffer.windowW, frameBuffer.windowH,
				1,
				32, // 一个像素占4个byte，32个bit
				BI_RGB,
				frameBuffer.windowW * frameBuffer.windowH * 4,
				0, 0, 0, 0
			}
		};
		LPVOID ptr = NULL;
		bitmap = CreateDIBSection(GetLibgOSDC(), &bitmapInfo, DIB_RGB_COLORS, &ptr, 0, 0);
		// 将bitmap设置为当前osdc的bitmap，操作bitmap即操作osdc
		oldBitmap = (HBITMAP)SelectObject(GetLibgOSDC(), bitmap);
		// 将frameBuffer的colorAttachment指向ptr，操作frameBuffer即操作bitmap
		frameBuffer.colorAttachment = (unsigned char*)ptr;
		frameBuffer.viewportH = frameBuffer.windowH;
		frameBuffer.viewportW = frameBuffer.windowW;

		// 申请深度缓冲，并将frameBuffer的depthAttachment指向它
		frameBuffer.depthAttachment = (float*)malloc(
			sizeof(float) * frameBuffer.windowW * frameBuffer.windowH);
	}

	// 改变frameBuffer的viewport相关变量
	//   各种边界检查
	frameBuffer.viewportX = x < 0 ? 0 : x;
	frameBuffer.viewportX = frameBuffer.viewportX >= frameBuffer.windowW - 1
		? frameBuffer.windowW - 1 : frameBuffer.viewportX;
	frameBuffer.viewportY = y < 0 ? 0 : y;
	frameBuffer.viewportY = frameBuffer.viewportY >= frameBuffer.windowH - 1
		? frameBuffer.windowH - 1 : frameBuffer.viewportY;
	int tmp;
	tmp = frameBuffer.viewportX + width;
	frameBuffer.viewportW = (tmp > frameBuffer.windowW ? frameBuffer.windowW : tmp)
		- frameBuffer.viewportX;
	tmp = frameBuffer.viewportY + height;
	frameBuffer.viewportH = (tmp > frameBuffer.windowH ? frameBuffer.windowH : tmp)
		- frameBuffer.viewportY;

	frameBuffer.viewportArea = (float)frameBuffer.viewportW * (float)frameBuffer.viewportH;

	// 声明新的视口后，肯定需要重绘
	markViewportNeedUpdate();
}

void libg3DClearWithColor(float r, float g, float b)
{
	// 步长，每跨一步相当于往上一行
	int stride = frameBuffer.windowW << 2;
	// 行起始指针，指向当前viewport 第N行 的 第一个像素
	unsigned char* rowPtr = frameBuffer.colorAttachment
		+ frameBuffer.viewportY * stride
		+ (frameBuffer.viewportX << 2);
	for (int row = 0; row < frameBuffer.viewportH; ++row)
	{
		// 像素起始指针，指向 rowPtr所在行 的 第N个 像素
		unsigned char* pixPtr = rowPtr;
		for (int col = 0; col < frameBuffer.viewportW; ++col)
		{
			// 逐像素赋颜色值
			// 注意：WIN32的BITMAP采用BGRA布局，而不是RGBA
			//   一个像素占4个byte，32个bit
			pixPtr[0] = colorFloatToUChar(b);
			pixPtr[1] = colorFloatToUChar(g);
			pixPtr[2] = colorFloatToUChar(r);
			pixPtr[3] = 255; // 透明度满格

			pixPtr += 4; // 往右一列
		}
		rowPtr += stride; // 往上一行
	}

	markViewportNeedUpdate();
}

void libg3DClearDepthBuffer()
{
	// 步长，每跨一步相当于往上一行
	int stride = frameBuffer.windowW;
	// 行起始指针，指向当前viewport 第N行 的 第一个像素
	float* rowPtr = frameBuffer.depthAttachment
		+ frameBuffer.viewportY * stride
		+ frameBuffer.viewportX;
	for (int row = 0; row < frameBuffer.viewportH; ++row)
	{
		// 像素起始指针，指向 rowPtr所在行 的 第N个 像素
		float* pixPtr = rowPtr;
		for (int col = 0; col < frameBuffer.viewportW; ++col)
		{
			// 逐像素赋深度值，赋裁剪空间的最大深度值
			*pixPtr = 1.f;

			pixPtr += 1; // 往右一列
		}
		rowPtr += stride; // 往上一行
	}

	markViewportNeedUpdate();
}

static float* MVPMat = NULL;
void libg3DApplyMVPMat(const float* const mat4x4Ptr[16])
{
	MVPMat = *mat4x4Ptr;
}

static struct Texture* texture = NULL;
void libg3DApplyTexture(const struct Texture* const* texPtr)
{
	if (texPtr == NULL)
		texture = NULL;
	else
		texture = *texPtr;
}

static enum TextureWarpMode texWarpMode = LIBG3D_TEXWARP_CLAMP_TO_EDGE;
void libg3DSetTextureWarpMode(enum TextureWarpMode warpMode)
{
	texWarpMode = warpMode;
}

static enum TextureFilterMode texMinFilterMode = LIBG3D_TEXFILT_LINEAR;
void libg3DSetTextureMinFilterMode(enum TextureFilterMode filtMode)
{
	texMinFilterMode = filtMode;
}

static enum TextureFilterMode texMagFilterMode = LIBG3D_TEXFILT_NEAREST;
void libg3DSetTextureMagFilterMode(enum TextureFilterMode filtMode)
{
	texMagFilterMode = filtMode;
}

struct VertexAttributeMeta
{
	unsigned int posNum;
	unsigned int colorNum;
	unsigned int texCoordNum;
};

static struct VertexAttributeMeta vertexAttributeMeta;

void libg3DSetVertAttributes(unsigned int posNum, unsigned int colorNum, unsigned int texCoordNum)
{
	if (posNum == 0 || posNum > 3 || colorNum > 4 || texCoordNum > 3
		|| (colorNum == 0 && texCoordNum == 0))
	{
		memset(&vertexAttributeMeta, 0, sizeof(vertexAttributeMeta));
		printf("[ERROR]Invalid vertex attribute number. In file: %s, at line: %d\n",
			__FILE__, __LINE__);
		return;
	}
	vertexAttributeMeta.posNum = posNum;
	vertexAttributeMeta.colorNum = colorNum;
	vertexAttributeMeta.texCoordNum = texCoordNum;
}

struct VertexShaderOutput
{
	float pos[4]; // xyzw
	union {
		float color[4]; // rgba
		float texCoord[2]; // uv
	};
};

// 图元预计支持从 点 到 三角面片，因此最大支持3个顶点
//   三角形立方体裁剪最多生成9个顶点
static struct VertexShaderOutput vertexShaderOutputs[9];

// 模拟顶点着色器的行为，计算结果储存在 vertexShaderOutputs 中
static void runVertexShader(const float* verts, unsigned int outputIdx)
{
	unsigned char idx = 0;
	float* dat = verts;
	while (idx != vertexAttributeMeta.posNum)
	{
		vertexShaderOutputs[outputIdx].pos[idx] = *dat;
		++dat;
		++idx;
	}
	while (idx < 3)
	{
		vertexShaderOutputs[outputIdx].pos[idx] = 0; // 不足补0
		++idx;
	}
	vertexShaderOutputs[outputIdx].pos[3] = 1.f; // w恒为1
	libg3DMat4x4MulVec4(
		vertexShaderOutputs[outputIdx].pos,
		MVPMat,
		vertexShaderOutputs[outputIdx].pos);

	if (texture == NULL)
	{
		// 渲染颜色，只记录颜色
		idx = 0;
		while (idx != vertexAttributeMeta.colorNum)
		{
			vertexShaderOutputs[outputIdx].color[idx] = *dat;
			++dat;
			++idx;
		}
		while (idx < 3)
		{
			vertexShaderOutputs[outputIdx].color[idx] = 0; // 不足补0
			++idx;
		}
		if (idx < 4)
			vertexShaderOutputs[outputIdx].color[3] = 1.f; // 未指定alpha则赋1
	}
	else
	{
		// 渲染纹理，只记录纹理坐标
		dat += vertexAttributeMeta.colorNum; // 跳过颜色数据
		idx = 0;
		while (idx != vertexAttributeMeta.texCoordNum)
		{
			vertexShaderOutputs[outputIdx].texCoord[idx] = *dat;
			++dat;
			++idx;
		}
		while (idx < 2)
		{
			vertexShaderOutputs[outputIdx].texCoord[idx] = 0; // 不足补0
			++idx;
		}
	}
}

// Liang-Barsky 线段裁剪算法，运行于裁剪空间，计算结果储存在 vertexShaderOutputs 中
static unsigned int runLiangBarskyAlgorithm()
{
	// 计算线段与裁剪区域的交集，用 进入步长tEnter 与 退出步长tExit 表示
	//   k = 0,1,2,3,4,5 代表 左、右、下、上、前、后 边界
	float tEnter = 0, tExit = 1.f; // 初始为整条线段
	float p, q;
	for (unsigned char k = 0; k < 6; ++k)
	{
		if (k == 4) continue; // 近切平面裁剪已执行，可跳过
		unsigned char xyz = k >> 1;
		if ((k & 0x1) == 0)
		{
			p = vertexShaderOutputs[0].pos[xyz] - vertexShaderOutputs[1].pos[xyz];
			q = vertexShaderOutputs[0].pos[xyz] - (-1.f);
		}
		else
		{
			p = vertexShaderOutputs[1].pos[xyz] - vertexShaderOutputs[0].pos[xyz];
			q = 1.f - vertexShaderOutputs[0].pos[xyz];
		}
		if (p == 0)
			if (q < 0) return 0; // 线段在k分量的投影完全在边界外
			else continue; // 线段在k分量的投影完全在边界内
		else if (p < 0)
			tEnter = fmaxf(fmaxf(0, q / p), tEnter);
		else
			tExit = fminf(fminf(1.f, q / p), tExit);
	}
	if (tEnter >= tExit) return 0;

	// 使用tEnter, tExit线性插值，获取新顶点及其属性
	float oldOri; // 起点
	float oldDlt; // 变化量
	//   k = 0,1,2 代表 x,y,z,w 四分量
	for (unsigned char k = 0; k < 4; ++k)
	{
		oldOri = vertexShaderOutputs[0].pos[k];
		oldDlt = vertexShaderOutputs[1].pos[k] - vertexShaderOutputs[0].pos[k];
		vertexShaderOutputs[0].pos[k] = oldOri + tEnter * oldDlt;
		vertexShaderOutputs[1].pos[k] = oldOri + tExit * oldDlt;
	}
	//   k = 0,1,2,3 代表 r,g,b,a 三分量
	for (unsigned char k = 0; k < 4; ++k)
	{
		oldOri = vertexShaderOutputs[0].color[k];
		oldDlt = vertexShaderOutputs[1].color[k] - vertexShaderOutputs[0].color[k];
		vertexShaderOutputs[0].color[k] = oldOri + tEnter * oldDlt;
		vertexShaderOutputs[1].color[k] = oldOri + tExit * oldDlt;
	}
	return 2;
}

// Sutherland-Hodgeman 三角形裁剪算法，运行于裁剪空间，计算结果储存在 vertexShaderOutputs 中
static unsigned int runSutherlandHodgemanAlgorithm()
{
	static struct VertexShaderOutput tmp[9];
	unsigned char validVertCnt = 3;

	// k = 0,1,2,3,4,5 代表 左、右、下、上、前、后 边界
	for (unsigned char k = 0; k < 6; ++k)
	{
		if (k == 4) continue; // 近切平面裁剪已执行，可跳过
		// 将上一轮裁剪的结果暂存入 tmp
		for (unsigned char k = 0; k < validVertCnt; ++k)
			tmp[k] = vertexShaderOutputs[k];
		// 根据SP与当前边界的关系，更新 vertexShaderOutputs
		unsigned char currValidVertCnt = 0;
		unsigned char xyz = k >> 1;
		unsigned char SIn, PIn;
		unsigned char S = validVertCnt - 1, P = 0;
		float p, q;
		for (; P != validVertCnt; S = P, ++P)
		{
			if ((k & 0x1) == 0)
			{
				SIn = tmp[S].pos[xyz] < -1.f ? 0 : 1;
				PIn = tmp[P].pos[xyz] < -1.f ? 0 : 1;
				p = tmp[S].pos[xyz] - tmp[P].pos[xyz];
				q = tmp[S].pos[xyz] - (-1.f);
			}
			else
			{
				SIn = tmp[S].pos[xyz] > 1.f ? 0 : 1;
				PIn = tmp[P].pos[xyz] > 1.f ? 0 : 1;
				p = tmp[P].pos[xyz] - tmp[S].pos[xyz];
				q = 1.f - tmp[S].pos[xyz];
			}

			if (SIn && PIn)
			{
				// S,P都不出界，添加P点
				vertexShaderOutputs[currValidVertCnt] = tmp[P];
				++currValidVertCnt;
			}
			else if (!SIn && !PIn); // S,P都出界，不添加点
			else
			{
				// SP有一个出界，计算 并 添加 SP与边界的交点
				float t = q / p;
				//   使用t线性插值
				//   k = 0,1,2 代表 x,y,z,w 四分量
				for (unsigned char k = 0; k < 4; ++k)
					vertexShaderOutputs[currValidVertCnt].pos[k] =
					tmp[S].pos[k] + t * (tmp[P].pos[k] - tmp[S].pos[k]);
				if (texture == NULL)
				{
					// 渲染颜色，只插值颜色
					//   k = 0,1,2,3 代表 r,g,b,a 三分量
					for (unsigned char k = 0; k < 4; ++k)
						vertexShaderOutputs[currValidVertCnt].color[k] =
						tmp[S].color[k] + t * (tmp[P].color[k] - tmp[S].color[k]);
				}
				else
				{
					// 渲染纹理，只插值纹理坐标
					//   k = 0,1 代表 u,v 二分量
					for (unsigned char k = 0; k < 2; ++k)
						vertexShaderOutputs[currValidVertCnt].texCoord[k] =
						tmp[S].texCoord[k] + t * (tmp[P].texCoord[k] - tmp[S].texCoord[k]);
				}
				++currValidVertCnt;

				if (!SIn)
				{
					// S出界P不出界，添加P
					vertexShaderOutputs[currValidVertCnt] = tmp[P];
					++currValidVertCnt;
				}
			}
		}
		validVertCnt = currValidVertCnt;
	}

	return validVertCnt;
}

struct FragmentShaderInput
{
	float depth;
	float rhw;
	union {
		float color[4]; // rgba
		float texCoord[2]; // uv
	};
};

static struct FragmentShaderInput* fragmentShaderInputs = NULL;
static struct VarArray_unsigned_int* activeFragments = NULL;

// Bresenham 线段光栅化算法，计算结果储存在 fragmentShaderInputs 中
static void runBresenhamAlgorithm()
{
	int col0 = (int)roundf(vertexShaderOutputs[0].pos[0]),
		row0 = (int)roundf(vertexShaderOutputs[0].pos[1]),
		col1 = (int)roundf(vertexShaderOutputs[1].pos[0]),
		row1 = (int)roundf(vertexShaderOutputs[1].pos[1]);
	int dCol = abs(col1 - col0),
		dRow = abs(row1 - row0);
	int sCol = col0 > col1 ? -1 : col0 == col1 ? 0 : 1,
		sRow = row0 > row1 ? -1 : row0 == row1 ? 0 : 1;
	int err;

	float t = 0, dt;

	struct FragmentShaderInput* FSIRowPtr = fragmentShaderInputs
		+ (frameBuffer.viewportY + row0) * frameBuffer.windowW
		+ frameBuffer.viewportX;
#define ATTR_LERP \
	FSIRowPtr[col0].color[0] = t * vertexShaderOutputs[1].color[0]\
		+ (1 - t) * vertexShaderOutputs[0].color[0];\
	FSIRowPtr[col0].color[1] = t * vertexShaderOutputs[1].color[1]\
		+ (1 - t) * vertexShaderOutputs[0].color[1];\
	FSIRowPtr[col0].color[2] = t * vertexShaderOutputs[1].color[2]\
		+ (1 - t) * vertexShaderOutputs[0].color[2];\
	FSIRowPtr[col0].color[3] = t * vertexShaderOutputs[1].color[3]\
		+ (1 - t) * vertexShaderOutputs[0].color[3];\
	FSIRowPtr[col0].depth = t * vertexShaderOutputs[1].pos[2]\
		+ (1 - t) * vertexShaderOutputs[0].pos[2];\
	FSIRowPtr[col0].rhw = t * vertexShaderOutputs[1].pos[3]\
		+ (1 - t) * vertexShaderOutputs[0].pos[3];

	if (dCol >= dRow)
	{
		// 若斜率小于等于1，列表带动行变
		//   让col0变化到col1
		err = -dCol;
		dt = 1.f / (float)dCol;
		while (col0 != col1)
		{
			ATTR_LERP;
			VarArray_unsigned_int_push_back(activeFragments,
				FSIRowPtr - fragmentShaderInputs + col0);

			col0 += sCol;
			t += dt;
			err += dRow << 1;
			if (err >= 0)
			{
				err -= dCol << 1;
				row0 += sRow;
				FSIRowPtr += sRow * frameBuffer.windowW;
			}
		}
	}
	else
	{
		// 若斜率大于1，行变带动列变
		//   让row0变化到row1
		err = -dRow;
		dt = 1.f / (float)dRow;
		while (row0 != row1)
		{
			ATTR_LERP;
			VarArray_unsigned_int_push_back(activeFragments,
				FSIRowPtr - fragmentShaderInputs + col0);

			row0 += sRow;
			FSIRowPtr += sRow * frameBuffer.windowW;
			t += dt;
			err += dCol << 1;
			if (err >= 0)
			{
				err -= dRow << 1;
				col0 += sCol;
			}
		}
	}

#undef ATTR_LERP
}

// 三角形光栅化算法，计算结果储存在 fragmentShaderInputs 中
static void runTriangleRasterization(unsigned int vertCnt)
{
	// 遍历所有裁剪出来的点组成的三角形
	unsigned char prev = 1, curr = 2;
	for (; curr != vertCnt; prev = curr, ++curr)
	{
		// 计算AABB包围盒
		int minRow, minCol, maxRow, maxCol;
		if (vertexShaderOutputs[0].pos[0] < vertexShaderOutputs[prev].pos[0])
		{
			minCol = (int)roundf(vertexShaderOutputs[0].pos[0]);
			maxCol = (int)roundf(vertexShaderOutputs[prev].pos[0]);
		}
		else
		{
			maxCol = (int)roundf(vertexShaderOutputs[0].pos[0]);
			minCol = (int)roundf(vertexShaderOutputs[prev].pos[0]);
		}
		if (minCol > vertexShaderOutputs[curr].pos[0])
			minCol = (int)roundf(vertexShaderOutputs[curr].pos[0]);
		if (maxCol < vertexShaderOutputs[curr].pos[0])
			maxCol = (int)roundf(vertexShaderOutputs[curr].pos[0]);

		if (vertexShaderOutputs[0].pos[1] < vertexShaderOutputs[prev].pos[1])
		{
			minRow = (int)roundf(vertexShaderOutputs[0].pos[1]);
			maxRow = (int)roundf(vertexShaderOutputs[prev].pos[1]);
		}
		else
		{
			maxRow = (int)roundf(vertexShaderOutputs[0].pos[1]);
			minRow = (int)roundf(vertexShaderOutputs[prev].pos[1]);
		}
		if (minRow > vertexShaderOutputs[curr].pos[1])
			minRow = (int)roundf(vertexShaderOutputs[curr].pos[1]);
		if (maxRow < vertexShaderOutputs[curr].pos[1])
			maxRow = (int)roundf(vertexShaderOutputs[curr].pos[1]);

		// 遍历AABB包围盒中的像素，激活三角形中的像素
		struct FragmentShaderInput* FSIRowPtr = fragmentShaderInputs
			+ (frameBuffer.viewportY + minRow) * frameBuffer.windowW
			+ frameBuffer.viewportX;
		float tmp0[3], tmp1[3], cross[3];
		tmp0[0] = vertexShaderOutputs[prev].pos[0] - vertexShaderOutputs[0].pos[0];
		tmp0[1] = vertexShaderOutputs[curr].pos[0] - vertexShaderOutputs[0].pos[0];
		tmp1[0] = vertexShaderOutputs[prev].pos[1] - vertexShaderOutputs[0].pos[1];
		tmp1[1] = vertexShaderOutputs[curr].pos[1] - vertexShaderOutputs[0].pos[1];
		for (int row = minRow; row <= maxRow; ++row)
		{
			for (int col = minCol; col <= maxCol; ++col)
			{
				tmp0[2] = vertexShaderOutputs[0].pos[0] - (float)col;
				tmp1[2] = vertexShaderOutputs[0].pos[1] - (float)row;
				libg3DVec3CrossVec3(cross, tmp0, tmp1);
				// cross[0,1,2] 分别储存顶点 0,prev,curr 对重心的权重
				cross[0] /= cross[2];
				cross[1] /= cross[2];
				cross[2] = 1.f - cross[0] - cross[1];

				// 像素不在三角形内部
				if (cross[0] < 0 || cross[0] > 1.f
					|| cross[1] < 0 || cross[1] > 1.f
					|| cross[2] < 0 || cross[2] > 1.f)
					continue;

				// 线性插值
				if (texture == NULL)
				{
					// 绘制颜色，因此只插值颜色
					FSIRowPtr[col].color[0] = cross[2] * vertexShaderOutputs[0].color[0]
						+ cross[0] * vertexShaderOutputs[prev].color[0]
						+ cross[1] * vertexShaderOutputs[curr].color[0];
					FSIRowPtr[col].color[1] = cross[2] * vertexShaderOutputs[0].color[1]
						+ cross[0] * vertexShaderOutputs[prev].color[1]
						+ cross[1] * vertexShaderOutputs[curr].color[1];
					FSIRowPtr[col].color[2] = cross[2] * vertexShaderOutputs[0].color[2]
						+ cross[0] * vertexShaderOutputs[prev].color[2]
						+ cross[1] * vertexShaderOutputs[curr].color[2];
					FSIRowPtr[col].color[3] = cross[2] * vertexShaderOutputs[0].color[3]
						+ cross[0] * vertexShaderOutputs[prev].color[3]
						+ cross[1] * vertexShaderOutputs[curr].color[3];
				}
				else
				{
					// 绘制纹理，只插值纹理坐标
					FSIRowPtr[col].texCoord[0] = cross[2] * vertexShaderOutputs[0].texCoord[0]
						+ cross[0] * vertexShaderOutputs[prev].texCoord[0]
						+ cross[1] * vertexShaderOutputs[curr].texCoord[0];
					FSIRowPtr[col].texCoord[1] = cross[2] * vertexShaderOutputs[0].texCoord[1]
						+ cross[0] * vertexShaderOutputs[prev].texCoord[1]
						+ cross[1] * vertexShaderOutputs[curr].texCoord[1];
				}
				// 深度
				FSIRowPtr[col].depth = cross[2] * vertexShaderOutputs[0].pos[2]
					+ cross[0] * vertexShaderOutputs[prev].pos[2]
					+ cross[1] * vertexShaderOutputs[curr].pos[2];
				// rhw
				FSIRowPtr[col].rhw = cross[2] * vertexShaderOutputs[0].pos[3]
					+ cross[0] * vertexShaderOutputs[prev].pos[3]
					+ cross[1] * vertexShaderOutputs[curr].pos[3];

				// 激活该片段
				VarArray_unsigned_int_push_back(activeFragments,
					FSIRowPtr - fragmentShaderInputs + col);
			}
			FSIRowPtr += frameBuffer.windowW;
		}
	}
}

// 模拟片段着色器行为，计算结果储存在 frameBuffer 中
static inline void runFragmentShader(unsigned int inputIdx)
{
	// 逐像素赋颜色值
	// 注意：WIN32的BITMAP采用BGRA布局，而不是RGBA
	//   一个像素占4个byte，32个bit
	unsigned char* pixPtr = frameBuffer.colorAttachment + (inputIdx << 2);
	if (texture == NULL)
	{
		// 绘制颜色
		pixPtr[0] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[2]);
		pixPtr[1] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[1]);
		pixPtr[2] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[0]);
		pixPtr[3] = 255;
	}
	else
		// 绘制纹理
		libg3DSampleTexture(
			pixPtr, texture,
			fragmentShaderInputs[inputIdx].texCoord[0], fragmentShaderInputs[inputIdx].texCoord[1],
			frameBuffer.viewportArea <= texture->area,
			texWarpMode,
			texMagFilterMode,
			texMinFilterMode);
	// 覆盖深度缓冲
	frameBuffer.depthAttachment[inputIdx] = fragmentShaderInputs[inputIdx].depth;
}

static enum LIBG3D_CULL_FACE_MODE cullFaceMode = LIBG3D_CULL_BACK;
void libg3DSetCullFaceMode(enum LIBG3D_CULL_FACE_MODE cullMode)
{
	cullFaceMode = cullMode;
}

static unsigned int* naturalIndices = NULL;
static unsigned int naturalIndicesNum = 0;

void libg3DDrawVerts(
		enum LIBG3D_PRIMITIVE primitive, const float* verts, unsigned int numOfVerts)
{
	// 绘制顶点数组 等价于 以 自然数数列0,1,2,3... 为索引绘制
	if (naturalIndices == NULL)
	{
		// 初始化
		naturalIndicesNum = numOfVerts;
		naturalIndices = (unsigned int*)malloc(sizeof(unsigned int) * naturalIndicesNum);
		for (unsigned int idx = 0; idx < naturalIndicesNum; ++idx)
			naturalIndices[idx] = idx;
	}
	else if (naturalIndicesNum < numOfVerts)
	{
		// 自动扩容
		while (naturalIndicesNum < numOfVerts)
			naturalIndicesNum <<= 1;
		free(naturalIndices);
		naturalIndices = (unsigned int*)malloc(sizeof(unsigned int) * naturalIndicesNum);
		for (unsigned int idx = 0; idx < naturalIndicesNum; ++idx)
			naturalIndices[idx] = idx;
	}

	libg3DDrawVertsByIndices(primitive, verts, naturalIndices, numOfVerts);
}

void libg3DDrawVertsByIndices(
		enum LIBG3D_PRIMITIVE primitive,
		const float* verts,
		const unsigned int* indices,
		unsigned int numOfIndices)
{
	if (MVPMat == NULL)
	{
		printf("[ERROR]MVPMat is NULL. In file: %s, at line: %d\n", __FILE__, __LINE__);
		return;
	}

	unsigned int grpVertNum;
	switch (primitive)
	{
	case LIBG3D_LINES:
		grpVertNum = 2; // 画一个线段，需2个顶点
		break;
	case LIBG3D_TRIANGLES:
		grpVertNum = 3; // 画一个三角形，需3个顶点
		break;
	default:
		printf("[ERROR]Invalid primitive. In file: %s, at line: %d\n", __FILE__, __LINE__);
		return;
	}

	size_t idxOffs = 0; // 当前顶点索引的内存地址偏移
	unsigned int attrLen = vertexAttributeMeta.posNum
		+ vertexAttributeMeta.colorNum
		+ vertexAttributeMeta.texCoordNum; // 当前每个顶点的属性所占大小
	// 按 图元 所需的顶点数分组，逐组运行
	for (unsigned int grpHeadIdx = 0; grpHeadIdx < numOfIndices; grpHeadIdx += grpVertNum)
	{
		// ---> 目前为世界空间 <---
		// 组内逐顶点运行顶点着色器
		for (unsigned int inGrpIdx = 0; inGrpIdx < grpVertNum; ++inGrpIdx)
		{
			runVertexShader(verts + attrLen * indices[idxOffs], inGrpIdx);
			++idxOffs;
		}
		// 近切平面裁剪
		//   TODO (太难了，不做了
		unsigned int validGrpVertCnt = grpVertNum;
		// 透视除法
		for (unsigned int inGrpIdx = 0; inGrpIdx < grpVertNum; ++inGrpIdx)
		{
			// 此时，w是 相机空间（View空间） 的z的负数，
			// 即顶点在相机空间的深度
			if (vertexShaderOutputs[inGrpIdx].pos[3] <= 0)
				// 若深度非正，抛弃该图元，作为没有近切平面裁剪的替代
				validGrpVertCnt = 0;
			// w分量储存rhw，rhw = 1/w = 1/(-z_view_space)
			vertexShaderOutputs[inGrpIdx].pos[3] = 1.f / vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[0] *= vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[1] *= vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[2] *= vertexShaderOutputs[inGrpIdx].pos[3];
			// 顶点属性的透视校正
			if (texture == NULL)
			{
				vertexShaderOutputs[inGrpIdx].color[0] *= vertexShaderOutputs[inGrpIdx].pos[3];
				vertexShaderOutputs[inGrpIdx].color[1] *= vertexShaderOutputs[inGrpIdx].pos[3];
				vertexShaderOutputs[inGrpIdx].color[2] *= vertexShaderOutputs[inGrpIdx].pos[3];
				vertexShaderOutputs[inGrpIdx].color[3] *= vertexShaderOutputs[inGrpIdx].pos[3];
			}
			else
			{
				vertexShaderOutputs[inGrpIdx].texCoord[0] *= vertexShaderOutputs[inGrpIdx].pos[3];
				vertexShaderOutputs[inGrpIdx].texCoord[1] *= vertexShaderOutputs[inGrpIdx].pos[3];
			}
		}
		if (validGrpVertCnt == 0)continue;
		// 面剔除
		if (primitive == LIBG3D_TRIANGLES && cullFaceMode != LIBG3D_CULL_NO_FACE)
		{
			float v0v1[3] = {
				vertexShaderOutputs[1].pos[0] - vertexShaderOutputs[0].pos[0],
				vertexShaderOutputs[1].pos[1] - vertexShaderOutputs[0].pos[1],
				vertexShaderOutputs[1].pos[2] - vertexShaderOutputs[0].pos[2]
			};
			float v0v2[3] = {
				vertexShaderOutputs[2].pos[0] - vertexShaderOutputs[0].pos[0],
				vertexShaderOutputs[2].pos[1] - vertexShaderOutputs[0].pos[1],
				vertexShaderOutputs[2].pos[2] - vertexShaderOutputs[0].pos[2]
			};
			// 计算法线
			libg3DVec3CrossVec3(v0v2, v0v1, v0v2);
			if (cullFaceMode == LIBG3D_CULL_BACK && v0v2[2] < 0)continue;
			else if (cullFaceMode == LIBG3D_CULL_FRONT && v0v2[2] >= 0)continue;
		}
		// ---> 目前为标准设备空间（裁剪空间） <---
		// 透视裁剪
		//   裁剪范围为[(-1,-1,-1), (1,1,1)]
		switch (primitive)
		{
		case LIBG3D_LINES:
			validGrpVertCnt = runLiangBarskyAlgorithm();
			break;
		case LIBG3D_TRIANGLES:
			validGrpVertCnt = runSutherlandHodgemanAlgorithm();
			break;
		default:
			printf("[ERROR]Invalid primitive. In file: %s, at line: %d\n", __FILE__, __LINE__);
			break;
		}
		if (validGrpVertCnt == 0)continue;
		// 视口变换
		for (unsigned int inGrpIdx = 0; inGrpIdx < validGrpVertCnt; ++inGrpIdx)
		{
			vertexShaderOutputs[inGrpIdx].pos[0] = (vertexShaderOutputs[inGrpIdx].pos[0] + 1.f) / 2.f * frameBuffer.viewportW;
			vertexShaderOutputs[inGrpIdx].pos[1] = (vertexShaderOutputs[inGrpIdx].pos[1] + 1.f) / 2.f * frameBuffer.viewportH;
		}

		// ---> 目前为屏幕空间 <---
		// 保证初始化只进行一次
		if (fragmentShaderInputs == NULL)
		{
			size_t size = sizeof(struct FragmentShaderInput)
				* frameBuffer.windowW * frameBuffer.windowH;
			fragmentShaderInputs = (struct FragmentShaderInput*)malloc(size);
			memset(fragmentShaderInputs, 0, size);
		}
		if (activeFragments == NULL)
			activeFragments = VarArray_unsigned_int_init(256);
		// 光栅化
		switch (primitive)
		{
		case LIBG3D_LINES:
			runBresenhamAlgorithm();
			break;
		case LIBG3D_TRIANGLES:
			runTriangleRasterization(validGrpVertCnt);
			break;
		default:
			printf("[ERROR]Invalid primitive. In file: %s, at line: %d\n", __FILE__, __LINE__);
			break;
		}
		// 逐片段运行片段着色器
		size_t finalRunFragments = 0;
		while (activeFragments->size != 0)
		{
			unsigned int idx = VarArray_unsigned_int_pop_back(activeFragments);
			// 进行提前深度测试，抛弃被遮挡片段
			if (fragmentShaderInputs[idx].depth > frameBuffer.depthAttachment[idx])
				continue; // 抛弃
			++finalRunFragments;
			// 顶点属性的透视校正（续）
			float w = 1.f / fragmentShaderInputs[idx].rhw;
			if (texture == NULL)
			{
				fragmentShaderInputs[idx].color[0] *= w;
				fragmentShaderInputs[idx].color[1] *= w;
				fragmentShaderInputs[idx].color[2] *= w;
				fragmentShaderInputs[idx].color[3] *= w;
			}
			else
			{
				fragmentShaderInputs[idx].texCoord[0] *= w;
				fragmentShaderInputs[idx].texCoord[1] *= w;
			}
			runFragmentShader(idx);
		}
		printf("[INFO]Fragment shader has processed %d fragments\n", finalRunFragments);
		// 深度测试
		//   暂时不需要
	}
	markViewportNeedUpdate();
	return;
}

void libg3DDestroy()
{
	if (bitmap != NULL)
	{
		SelectObject(GetLibgOSDC(), oldBitmap);
		DeleteObject(bitmap);
		bitmap = oldBitmap = NULL;
	}
	if (fragmentShaderInputs != NULL)
	{
		free(fragmentShaderInputs);
		fragmentShaderInputs = NULL;
	}
	if (activeFragments != NULL)
	{
		VarArray_unsigned_int_destroy(activeFragments);
		activeFragments = NULL;
	}
}
