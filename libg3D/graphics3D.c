#include "graphics3D.h"

#include "varArray.h"

#include "lib3DExport.h"
#include "../libgraphics/win32Export.h"

#include <math.h>

// WIN32Ԫ��
static HBITMAP oldBitmap = NULL, bitmap = NULL;

// ֡����ṹ
//   һ�����ھ���һ��֡����ٿء�
struct FrameBuffer
{
	int windowW, windowH; // ���ڴ�С
	int viewportX, viewportY; // �ӿھ��봰�����½ǵ�ƫ��
	int viewportW, viewportH; // �ӿڴ�С
	float viewportArea; // �ӿ����
	unsigned char* colorAttachment; // ֡�������ɫ����
	float* depthAttachment; // ֡�������Ȼ���
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
	// ������viewport���Ϊ��Ҫ���£�������Ҫ������
	// ע�⣺viewportRECTʹ��Win32����ϵ��
	//   y�ᷴ��ͬʱԭ���ڴ������Ͻ�
	RECT viewportRECT = {
		frameBuffer.viewportX, // ��
		frameBuffer.windowH - frameBuffer.viewportY - frameBuffer.viewportH, // ��
		frameBuffer.viewportX + frameBuffer.viewportW, // ��
		frameBuffer.windowH - frameBuffer.viewportY // ��
	};
	InvalidateRect(GetLibgWindow(), &viewportRECT, FALSE);
}

void libg3DViewport(int x, int y, int width, int height)
{
	// ��֤��ʼ��ֻ����һ��
	if (frameBuffer.colorAttachment == NULL)
	{
		// ��ȡgraphics.c�����Ĵ��ڳߴ�
		frameBuffer.windowW = GetLibgWindowPixelWidth();
		frameBuffer.windowH = GetLibgWindowPixelHeight();
		// �½�һ����osdc�����bitmap����
		//   BITMAPINFO ���
		//   https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo
		BITMAPINFO bitmapInfo = { {
				sizeof(BITMAPINFOHEADER),
				frameBuffer.windowW, frameBuffer.windowH,
				1,
				32, // һ������ռ4��byte��32��bit
				BI_RGB,
				frameBuffer.windowW * frameBuffer.windowH * 4,
				0, 0, 0, 0
			}
		};
		LPVOID ptr = NULL;
		bitmap = CreateDIBSection(GetLibgOSDC(), &bitmapInfo, DIB_RGB_COLORS, &ptr, 0, 0);
		// ��bitmap����Ϊ��ǰosdc��bitmap������bitmap������osdc
		oldBitmap = (HBITMAP)SelectObject(GetLibgOSDC(), bitmap);
		// ��frameBuffer��colorAttachmentָ��ptr������frameBuffer������bitmap
		frameBuffer.colorAttachment = (unsigned char*)ptr;
		frameBuffer.viewportH = frameBuffer.windowH;
		frameBuffer.viewportW = frameBuffer.windowW;

		// ������Ȼ��壬����frameBuffer��depthAttachmentָ����
		frameBuffer.depthAttachment = (float*)malloc(
			sizeof(float) * frameBuffer.windowW * frameBuffer.windowH);
	}

	// �ı�frameBuffer��viewport��ر���
	//   ���ֱ߽���
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

	// �����µ��ӿں󣬿϶���Ҫ�ػ�
	markViewportNeedUpdate();
}

void libg3DClearWithColor(float r, float g, float b)
{
	// ������ÿ��һ���൱������һ��
	int stride = frameBuffer.windowW << 2;
	// ����ʼָ�룬ָ��ǰviewport ��N�� �� ��һ������
	unsigned char* rowPtr = frameBuffer.colorAttachment
		+ frameBuffer.viewportY * stride
		+ (frameBuffer.viewportX << 2);
	for (int row = 0; row < frameBuffer.viewportH; ++row)
	{
		// ������ʼָ�룬ָ�� rowPtr������ �� ��N�� ����
		unsigned char* pixPtr = rowPtr;
		for (int col = 0; col < frameBuffer.viewportW; ++col)
		{
			// �����ظ���ɫֵ
			// ע�⣺WIN32��BITMAP����BGRA���֣�������RGBA
			//   һ������ռ4��byte��32��bit
			pixPtr[0] = colorFloatToUChar(b);
			pixPtr[1] = colorFloatToUChar(g);
			pixPtr[2] = colorFloatToUChar(r);
			pixPtr[3] = 255; // ͸��������

			pixPtr += 4; // ����һ��
		}
		rowPtr += stride; // ����һ��
	}

	markViewportNeedUpdate();
}

void libg3DClearDepthBuffer()
{
	// ������ÿ��һ���൱������һ��
	int stride = frameBuffer.windowW;
	// ����ʼָ�룬ָ��ǰviewport ��N�� �� ��һ������
	float* rowPtr = frameBuffer.depthAttachment
		+ frameBuffer.viewportY * stride
		+ frameBuffer.viewportX;
	for (int row = 0; row < frameBuffer.viewportH; ++row)
	{
		// ������ʼָ�룬ָ�� rowPtr������ �� ��N�� ����
		float* pixPtr = rowPtr;
		for (int col = 0; col < frameBuffer.viewportW; ++col)
		{
			// �����ظ����ֵ�����ü��ռ��������ֵ
			*pixPtr = 1.f;

			pixPtr += 1; // ����һ��
		}
		rowPtr += stride; // ����һ��
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

// ͼԪԤ��֧�ִ� �� �� ������Ƭ��������֧��3������
//   ������������ü��������9������
static struct VertexShaderOutput vertexShaderOutputs[9];

// ģ�ⶥ����ɫ������Ϊ�������������� vertexShaderOutputs ��
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
		vertexShaderOutputs[outputIdx].pos[idx] = 0; // ���㲹0
		++idx;
	}
	vertexShaderOutputs[outputIdx].pos[3] = 1.f; // w��Ϊ1
	libg3DMat4x4MulVec4(
		vertexShaderOutputs[outputIdx].pos,
		MVPMat,
		vertexShaderOutputs[outputIdx].pos);

	if (texture == NULL)
	{
		// ��Ⱦ��ɫ��ֻ��¼��ɫ
		idx = 0;
		while (idx != vertexAttributeMeta.colorNum)
		{
			vertexShaderOutputs[outputIdx].color[idx] = *dat;
			++dat;
			++idx;
		}
		while (idx < 3)
		{
			vertexShaderOutputs[outputIdx].color[idx] = 0; // ���㲹0
			++idx;
		}
		if (idx < 4)
			vertexShaderOutputs[outputIdx].color[3] = 1.f; // δָ��alpha��1
	}
	else
	{
		// ��Ⱦ����ֻ��¼��������
		dat += vertexAttributeMeta.colorNum; // ������ɫ����
		idx = 0;
		while (idx != vertexAttributeMeta.texCoordNum)
		{
			vertexShaderOutputs[outputIdx].texCoord[idx] = *dat;
			++dat;
			++idx;
		}
		while (idx < 2)
		{
			vertexShaderOutputs[outputIdx].texCoord[idx] = 0; // ���㲹0
			++idx;
		}
	}
}

// Liang-Barsky �߶βü��㷨�������ڲü��ռ䣬������������ vertexShaderOutputs ��
static unsigned int runLiangBarskyAlgorithm()
{
	// �����߶���ü�����Ľ������� ���벽��tEnter �� �˳�����tExit ��ʾ
	//   k = 0,1,2,3,4,5 ���� ���ҡ��¡��ϡ�ǰ���� �߽�
	float tEnter = 0, tExit = 1.f; // ��ʼΪ�����߶�
	float p, q;
	for (unsigned char k = 0; k < 6; ++k)
	{
		if (k == 4) continue; // ����ƽ��ü���ִ�У�������
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
			if (q < 0) return 0; // �߶���k������ͶӰ��ȫ�ڱ߽���
			else continue; // �߶���k������ͶӰ��ȫ�ڱ߽���
		else if (p < 0)
			tEnter = fmaxf(fmaxf(0, q / p), tEnter);
		else
			tExit = fminf(fminf(1.f, q / p), tExit);
	}
	if (tEnter >= tExit) return 0;

	// ʹ��tEnter, tExit���Բ�ֵ����ȡ�¶��㼰������
	float oldOri; // ���
	float oldDlt; // �仯��
	//   k = 0,1,2 ���� x,y,z,w �ķ���
	for (unsigned char k = 0; k < 4; ++k)
	{
		oldOri = vertexShaderOutputs[0].pos[k];
		oldDlt = vertexShaderOutputs[1].pos[k] - vertexShaderOutputs[0].pos[k];
		vertexShaderOutputs[0].pos[k] = oldOri + tEnter * oldDlt;
		vertexShaderOutputs[1].pos[k] = oldOri + tExit * oldDlt;
	}
	//   k = 0,1,2,3 ���� r,g,b,a ������
	for (unsigned char k = 0; k < 4; ++k)
	{
		oldOri = vertexShaderOutputs[0].color[k];
		oldDlt = vertexShaderOutputs[1].color[k] - vertexShaderOutputs[0].color[k];
		vertexShaderOutputs[0].color[k] = oldOri + tEnter * oldDlt;
		vertexShaderOutputs[1].color[k] = oldOri + tExit * oldDlt;
	}
	return 2;
}

// Sutherland-Hodgeman �����βü��㷨�������ڲü��ռ䣬������������ vertexShaderOutputs ��
static unsigned int runSutherlandHodgemanAlgorithm()
{
	static struct VertexShaderOutput tmp[9];
	unsigned char validVertCnt = 3;

	// k = 0,1,2,3,4,5 ���� ���ҡ��¡��ϡ�ǰ���� �߽�
	for (unsigned char k = 0; k < 6; ++k)
	{
		if (k == 4) continue; // ����ƽ��ü���ִ�У�������
		// ����һ�ֲü��Ľ���ݴ��� tmp
		for (unsigned char k = 0; k < validVertCnt; ++k)
			tmp[k] = vertexShaderOutputs[k];
		// ����SP�뵱ǰ�߽�Ĺ�ϵ������ vertexShaderOutputs
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
				// S,P�������磬���P��
				vertexShaderOutputs[currValidVertCnt] = tmp[P];
				++currValidVertCnt;
			}
			else if (!SIn && !PIn); // S,P�����磬����ӵ�
			else
			{
				// SP��һ�����磬���� �� ��� SP��߽�Ľ���
				float t = q / p;
				//   ʹ��t���Բ�ֵ
				//   k = 0,1,2 ���� x,y,z,w �ķ���
				for (unsigned char k = 0; k < 4; ++k)
					vertexShaderOutputs[currValidVertCnt].pos[k] =
					tmp[S].pos[k] + t * (tmp[P].pos[k] - tmp[S].pos[k]);
				if (texture == NULL)
				{
					// ��Ⱦ��ɫ��ֻ��ֵ��ɫ
					//   k = 0,1,2,3 ���� r,g,b,a ������
					for (unsigned char k = 0; k < 4; ++k)
						vertexShaderOutputs[currValidVertCnt].color[k] =
						tmp[S].color[k] + t * (tmp[P].color[k] - tmp[S].color[k]);
				}
				else
				{
					// ��Ⱦ����ֻ��ֵ��������
					//   k = 0,1 ���� u,v ������
					for (unsigned char k = 0; k < 2; ++k)
						vertexShaderOutputs[currValidVertCnt].texCoord[k] =
						tmp[S].texCoord[k] + t * (tmp[P].texCoord[k] - tmp[S].texCoord[k]);
				}
				++currValidVertCnt;

				if (!SIn)
				{
					// S����P�����磬���P
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

// Bresenham �߶ι�դ���㷨�������������� fragmentShaderInputs ��
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
		// ��б��С�ڵ���1���б�����б�
		//   ��col0�仯��col1
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
		// ��б�ʴ���1���б�����б�
		//   ��row0�仯��row1
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

// �����ι�դ���㷨�������������� fragmentShaderInputs ��
static void runTriangleRasterization(unsigned int vertCnt)
{
	// �������вü������ĵ���ɵ�������
	unsigned char prev = 1, curr = 2;
	for (; curr != vertCnt; prev = curr, ++curr)
	{
		// ����AABB��Χ��
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

		// ����AABB��Χ���е����أ������������е�����
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
				// cross[0,1,2] �ֱ𴢴涥�� 0,prev,curr �����ĵ�Ȩ��
				cross[0] /= cross[2];
				cross[1] /= cross[2];
				cross[2] = 1.f - cross[0] - cross[1];

				// ���ز����������ڲ�
				if (cross[0] < 0 || cross[0] > 1.f
					|| cross[1] < 0 || cross[1] > 1.f
					|| cross[2] < 0 || cross[2] > 1.f)
					continue;

				// ���Բ�ֵ
				if (texture == NULL)
				{
					// ������ɫ�����ֻ��ֵ��ɫ
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
					// ��������ֻ��ֵ��������
					FSIRowPtr[col].texCoord[0] = cross[2] * vertexShaderOutputs[0].texCoord[0]
						+ cross[0] * vertexShaderOutputs[prev].texCoord[0]
						+ cross[1] * vertexShaderOutputs[curr].texCoord[0];
					FSIRowPtr[col].texCoord[1] = cross[2] * vertexShaderOutputs[0].texCoord[1]
						+ cross[0] * vertexShaderOutputs[prev].texCoord[1]
						+ cross[1] * vertexShaderOutputs[curr].texCoord[1];
				}
				// ���
				FSIRowPtr[col].depth = cross[2] * vertexShaderOutputs[0].pos[2]
					+ cross[0] * vertexShaderOutputs[prev].pos[2]
					+ cross[1] * vertexShaderOutputs[curr].pos[2];
				// rhw
				FSIRowPtr[col].rhw = cross[2] * vertexShaderOutputs[0].pos[3]
					+ cross[0] * vertexShaderOutputs[prev].pos[3]
					+ cross[1] * vertexShaderOutputs[curr].pos[3];

				// �����Ƭ��
				VarArray_unsigned_int_push_back(activeFragments,
					FSIRowPtr - fragmentShaderInputs + col);
			}
			FSIRowPtr += frameBuffer.windowW;
		}
	}
}

// ģ��Ƭ����ɫ����Ϊ�������������� frameBuffer ��
static inline void runFragmentShader(unsigned int inputIdx)
{
	// �����ظ���ɫֵ
	// ע�⣺WIN32��BITMAP����BGRA���֣�������RGBA
	//   һ������ռ4��byte��32��bit
	unsigned char* pixPtr = frameBuffer.colorAttachment + (inputIdx << 2);
	if (texture == NULL)
	{
		// ������ɫ
		pixPtr[0] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[2]);
		pixPtr[1] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[1]);
		pixPtr[2] = colorFloatToUChar(fragmentShaderInputs[inputIdx].color[0]);
		pixPtr[3] = 255;
	}
	else
		// ��������
		libg3DSampleTexture(
			pixPtr, texture,
			fragmentShaderInputs[inputIdx].texCoord[0], fragmentShaderInputs[inputIdx].texCoord[1],
			frameBuffer.viewportArea <= texture->area,
			texWarpMode,
			texMagFilterMode,
			texMinFilterMode);
	// ������Ȼ���
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
	// ���ƶ������� �ȼ��� �� ��Ȼ������0,1,2,3... Ϊ��������
	if (naturalIndices == NULL)
	{
		// ��ʼ��
		naturalIndicesNum = numOfVerts;
		naturalIndices = (unsigned int*)malloc(sizeof(unsigned int) * naturalIndicesNum);
		for (unsigned int idx = 0; idx < naturalIndicesNum; ++idx)
			naturalIndices[idx] = idx;
	}
	else if (naturalIndicesNum < numOfVerts)
	{
		// �Զ�����
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
		grpVertNum = 2; // ��һ���߶Σ���2������
		break;
	case LIBG3D_TRIANGLES:
		grpVertNum = 3; // ��һ�������Σ���3������
		break;
	default:
		printf("[ERROR]Invalid primitive. In file: %s, at line: %d\n", __FILE__, __LINE__);
		return;
	}

	size_t idxOffs = 0; // ��ǰ�����������ڴ��ַƫ��
	unsigned int attrLen = vertexAttributeMeta.posNum
		+ vertexAttributeMeta.colorNum
		+ vertexAttributeMeta.texCoordNum; // ��ǰÿ�������������ռ��С
	// �� ͼԪ ����Ķ��������飬��������
	for (unsigned int grpHeadIdx = 0; grpHeadIdx < numOfIndices; grpHeadIdx += grpVertNum)
	{
		// ---> ĿǰΪ����ռ� <---
		// �����𶥵����ж�����ɫ��
		for (unsigned int inGrpIdx = 0; inGrpIdx < grpVertNum; ++inGrpIdx)
		{
			runVertexShader(verts + attrLen * indices[idxOffs], inGrpIdx);
			++idxOffs;
		}
		// ����ƽ��ü�
		//   TODO (̫���ˣ�������
		unsigned int validGrpVertCnt = grpVertNum;
		// ͸�ӳ���
		for (unsigned int inGrpIdx = 0; inGrpIdx < grpVertNum; ++inGrpIdx)
		{
			// ��ʱ��w�� ����ռ䣨View�ռ䣩 ��z�ĸ�����
			// ������������ռ�����
			if (vertexShaderOutputs[inGrpIdx].pos[3] <= 0)
				// ����ȷ�����������ͼԪ����Ϊû�н���ƽ��ü������
				validGrpVertCnt = 0;
			// w��������rhw��rhw = 1/w = 1/(-z_view_space)
			vertexShaderOutputs[inGrpIdx].pos[3] = 1.f / vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[0] *= vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[1] *= vertexShaderOutputs[inGrpIdx].pos[3];
			vertexShaderOutputs[inGrpIdx].pos[2] *= vertexShaderOutputs[inGrpIdx].pos[3];
			// �������Ե�͸��У��
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
		// ���޳�
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
			// ���㷨��
			libg3DVec3CrossVec3(v0v2, v0v1, v0v2);
			if (cullFaceMode == LIBG3D_CULL_BACK && v0v2[2] < 0)continue;
			else if (cullFaceMode == LIBG3D_CULL_FRONT && v0v2[2] >= 0)continue;
		}
		// ---> ĿǰΪ��׼�豸�ռ䣨�ü��ռ䣩 <---
		// ͸�Ӳü�
		//   �ü���ΧΪ[(-1,-1,-1), (1,1,1)]
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
		// �ӿڱ任
		for (unsigned int inGrpIdx = 0; inGrpIdx < validGrpVertCnt; ++inGrpIdx)
		{
			vertexShaderOutputs[inGrpIdx].pos[0] = (vertexShaderOutputs[inGrpIdx].pos[0] + 1.f) / 2.f * frameBuffer.viewportW;
			vertexShaderOutputs[inGrpIdx].pos[1] = (vertexShaderOutputs[inGrpIdx].pos[1] + 1.f) / 2.f * frameBuffer.viewportH;
		}

		// ---> ĿǰΪ��Ļ�ռ� <---
		// ��֤��ʼ��ֻ����һ��
		if (fragmentShaderInputs == NULL)
		{
			size_t size = sizeof(struct FragmentShaderInput)
				* frameBuffer.windowW * frameBuffer.windowH;
			fragmentShaderInputs = (struct FragmentShaderInput*)malloc(size);
			memset(fragmentShaderInputs, 0, size);
		}
		if (activeFragments == NULL)
			activeFragments = VarArray_unsigned_int_init(256);
		// ��դ��
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
		// ��Ƭ������Ƭ����ɫ��
		size_t finalRunFragments = 0;
		while (activeFragments->size != 0)
		{
			unsigned int idx = VarArray_unsigned_int_pop_back(activeFragments);
			// ������ǰ��Ȳ��ԣ��������ڵ�Ƭ��
			if (fragmentShaderInputs[idx].depth > frameBuffer.depthAttachment[idx])
				continue; // ����
			++finalRunFragments;
			// �������Ե�͸��У��������
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
		// ��Ȳ���
		//   ��ʱ����Ҫ
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
