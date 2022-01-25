#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <stdio.h>
#include <stdlib.h>

enum TextureFormat
{
	LIBG3D_TEXFORM_R,
	LIBG3D_TEXFORM_RGB,
	LIBG3D_TEXFORM_RGBA
};

enum TextureWarpMode
{
	LIBG3D_TEXWARP_CLAMP_TO_EDGE
};

enum TextureFilterMode
{
	LIBG3D_TEXFILT_NEAREST,
	LIBG3D_TEXFILT_LINEAR
};

struct Texture
{
	unsigned int w, h, channel;
	float area;
	enum TextureFormat fmt;
	unsigned char* dat;
};

inline unsigned char colorFloatToUChar(float f)
{
	f *= 255.f;
	if (f < 0)return 0;
	else if (f >= 255.f)return 255;
	return (unsigned char)roundf(f);
}

/// <summary>
/// 创建一个纹理对象
/// </summary>
/// <param name="imgDat">用于创建纹理的图像数据（8位深度）</param>
/// <param name="width">图像宽度</param>
/// <param name="height">图像高度（8位深度）</param>
/// <param name="format">图像、纹理的通道格式</param>
static struct Texture* libg3DGenTexture(
	const unsigned char* imgDat,
	unsigned int width, unsigned int height,
	enum TextureFormat format)
{
	struct Texture* tex = (struct Texture*)malloc(sizeof(struct Texture));
	tex->w = width;
	tex->h = height;
	tex->area = (float)tex->w * (float)tex->h;
	tex->fmt = format;
	switch (tex->fmt)
	{
	case LIBG3D_TEXFORM_R:
		tex->channel = 1;
		break;
	case LIBG3D_TEXFORM_RGB:
		tex->channel = 3;
		break;
	case LIBG3D_TEXFORM_RGBA:
		tex->channel = 4;
		break;
	default:
		break;
	}
	size_t size = sizeof(unsigned char) * tex->w * tex->h * tex->channel;
	tex->dat = (unsigned char*)malloc(size);
	memcpy(tex->dat, imgDat, size);
	
	return tex;
}

static void libg3DDestroyTexture(struct Texture* tex)
{
	free(tex->dat);
	free(tex);
}

/// <summary>
/// 从纹理对象tex采样得到颜色值rgba
/// </summary>
/// <param name="rgba">采样结果</param>
/// <param name="tex">纹理对象</param>
/// <param name="u">纹理横坐标</param>
/// <param name="v">纹理纵坐标</param>
/// <param name="texWarpMode">越界纹理坐标对应模式</param>
/// <param name="texMagFilterMode">放大过滤模式</param>
/// <param name="texMinFilterMode">缩小过滤模式</param>
static inline void libg3DSampleTexture(
	unsigned char* bgra,
	const struct Texture* tex,
	float u, float v, unsigned char isMag,
	enum TextureWarpMode texWarpMode,
	enum TextureFilterMode texMagFilterMode,
	enum TextureFilterMode texMinFilterMode)
{
	switch (texWarpMode)
	{
	case LIBG3D_TEXWARP_CLAMP_TO_EDGE:
		u = u < 0 ? 0 : u > 1.f ? 1.f : u;
		v = v < 0 ? 0 : v > 1.f ? 1.f : v;
		break;
	default:
		printf("[ERROR]Invalid texWarpMode. In file: %s, at line: %d\n", __FILE__, __LINE__);
		break;
	}

#define SAMPLE(bgra) \
	switch (tex->fmt)\
	{\
	case LIBG3D_TEXFORM_RGBA:\
		##bgra[2] = tex->dat[flatIdx + 0];\
		##bgra[1] = tex->dat[flatIdx + 1];\
		##bgra[0] = tex->dat[flatIdx + 2];\
		##bgra[3] = tex->dat[flatIdx + 3];\
		break;\
	case LIBG3D_TEXFORM_RGB:\
		##bgra[2] = tex->dat[flatIdx + 0];\
		##bgra[1] = tex->dat[flatIdx + 1];\
		##bgra[0] = tex->dat[flatIdx + 2];\
		##bgra[3] = 255;\
		break;\
	case LIBG3D_TEXFORM_R:\
		##bgra[2] = tex->dat[flatIdx + 0];\
		##bgra[1] = 0;\
		##bgra[0] = 0;\
		##bgra[3] = 255;\
		break;\
	default:\
		printf("[ERROR]Invalid texture format. In file: %s, at line: %d\n", __FILE__, __LINE__);\
		break;\
	}

	size_t flatIdx;
	enum TextureFilterMode filterMode = isMag != 0 ? texMagFilterMode : texMinFilterMode;
	if (filterMode == LIBG3D_TEXFILT_NEAREST)
	{
		size_t row = (size_t)floorf(v * (float)tex->h);
		size_t col = (size_t)floorf(u * (float)tex->w);
		flatIdx = row * tex->w + col;
		flatIdx <<= 2;
		SAMPLE(bgra);
	}
	else if (filterMode == LIBG3D_TEXFILT_LINEAR)
	{
		float tmp[4][4], t[2];
		float uw = u * (float)tex->w;
		float vh = v * (float)tex->h;
		size_t col = (size_t)floorf(uw);
		size_t row = (size_t)floorf(vh);
		flatIdx = row * tex->w + col;
		flatIdx <<= 2;
		if (col == 0 || col == tex->w - 1 || row == 0 || row == tex->h - 1)
		{
			SAMPLE(bgra);
		}
		else
		{
			SAMPLE(tmp[0]);

			t[0] = uw - (float)col - .5f;
			t[1] = vh - (float)row - .5f;
			size_t lftBotFlatIdx = flatIdx;
			unsigned int channel = 0;
			if (t[0] >= 0)
			{
				flatIdx = lftBotFlatIdx + tex->channel; // 取右体素
				SAMPLE(tmp[1]);
				if (t[1] >= 0)
				{
					flatIdx = lftBotFlatIdx + tex->w * tex->channel; // 取上体素
					SAMPLE(tmp[2]);
					flatIdx += tex->channel; // 取右上体素
					SAMPLE(tmp[3]);
				}
				else
				{
					t[1] = -t[1];
					flatIdx = lftBotFlatIdx - tex->w * tex->channel; // 取下体素
					SAMPLE(tmp[2]);
					flatIdx += tex->channel; // 取右下体素
					SAMPLE(tmp[3]);
				}
			}
			else
			{
				flatIdx = lftBotFlatIdx - tex->channel; // 取左体素
				SAMPLE(tmp[1]);
				t[0] = -t[0];
				if (t[1] >= 0)
				{
					flatIdx = lftBotFlatIdx + tex->w * tex->channel; // 取上体素
					SAMPLE(tmp[2]);
					flatIdx -= tex->channel; // 取左上体素
					SAMPLE(tmp[3]);
				}
				else
				{
					t[1] = -t[1];
					flatIdx = lftBotFlatIdx - tex->w * tex->channel; // 取下体素
					SAMPLE(tmp[2]);
					flatIdx -= tex->channel; // 取左下体素
					SAMPLE(tmp[3]);
				}
			}

			// 双线性插值
#define BILERP(attrIdx) \
			bgra[##attrIdx] = t[1] * (t[0] * tmp[3][attrIdx] + (1 - t[0]) * tmp[2][attrIdx])\
				+ (1 - t[1]) * (t[0] * tmp[1][attrIdx] + (1 - t[0]) * tmp[0][attrIdx]);

			BILERP(0);
			BILERP(1);
			BILERP(2);
			BILERP(3);
#undef BILERP
		}
	}
	else if (isMag != 0)
		printf("[ERROR]Invalid texMagFilterMode. In file: %s, at line: %d\n", __FILE__, __LINE__);
	else
		printf("[ERROR]Invalid texMinFilterMode. In file: %s, at line: %d\n", __FILE__, __LINE__);

#undef SAMPLE
}

#endif // !_TEXTURE_H
