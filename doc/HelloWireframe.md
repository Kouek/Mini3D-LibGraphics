# 画一个线框立方体

[toc]

## 例子程序

```c
#include <math.h>

#include "libgraphics/extgraph.h"
#include "libg3D/graphics3D.h"

#include <Windows.h>

// 线框立方体顶点编号
//    2 +-----+ 3
//     /|    /|     y
//    / |   / |    /|\
// 6 +-----+ 7|	    |
//   |0 +--|--+ 1   o------>x
//   | /   | /     /
//   |/    |/    |/_
// 4 +-----+ 5   z
static float cubeVerts[8 * 6];
static unsigned int cubeIndices[12 * 2] = {
	// 后面
	0, 1, 1, 3, 3, 2, 2, 0,
	// 前面
	4, 5, 5, 7, 7, 6, 6, 4,
	// 侧面
	0, 4, 2, 6, 1, 5, 3, 7
};

// 模型、相机位姿参数
static float orbitZOXAngleDeg = 0;
static float orbitYAngleDeg = 0;
static float dist = 1.f;
// 投影参数
static float nearClip = .1f;
static float FOV = 20.f;
static float aspectHW = 1.f;

static float changed = 1;

static void keyboardCallback(int key, int event)
{
	if (event == KEY_DOWN)
		if (key == VK_RIGHT)
		{
			orbitYAngleDeg += 10.f;
			changed = 1;
		}
		else if (key == VK_LEFT)
		{
			orbitYAngleDeg -= 10.f;
			changed = 1;
		}
		else if (key == VK_UP)
		{
			orbitZOXAngleDeg += 2.f;
			changed = 1;
		}
		else if (key == VK_DOWN)
		{
			orbitZOXAngleDeg -= 2.f;
			changed = 1;
		}
		else if (key == 'Q')
		{
			dist -= 0.01f;
			changed = 1;
		}
		else if (key == 'E')
		{
			dist += 0.01f;
			changed = 1;
		}
		else if (key == 'N')
		{
			nearClip -= 0.01f;
			changed = 1;
		}
		else if (key == 'M')
		{
			nearClip += 0.01f;
			changed = 1;
		}
		else if (key == 'F')
		{
			FOV -= 5.f;
			changed = 1;
		}
		else if (key == 'G')
		{
			FOV += 5.f;
			changed = 1;
		}
		else if (key == 'A')
		{
			aspectHW -= .1f;
			changed = 1;
		}
		else if (key == 'S')
		{
			aspectHW += .1f;
			changed = 1;
		}
}

static void timerCallback(int timerID)
{
	// 减少逐帧运算量
	if (changed == 0)return;
	changed = 0;

	// 设置视口
	unsigned int pixW = libg3DGetWindowPixelWidth();
	unsigned int pixH = libg3DGetWindowPixelHeight();
	libg3DViewport(pixW / 8, pixH / 8, pixW * 3 / 4, pixH * 3 / 4);

	// 每个顶点有6个float长的属性，前3为位置向量，后3为颜色向量
	libg3DSetVertAttributes(3, 3, 0);

	// 设置MVP
	//  设置模型矩阵为单位阵
	static float* model = NULL;
	if (model == NULL)
		model = libg3DGenIdentity4x4(); // 内存漏了但不要紧
	//   设置一个环绕相机
	//     初始化环绕中心点为原点
	static float center[3] = { 0,0,0 };
	//     计算当前方向
    static float drc[3];
	drc[0] = cosf(orbitZOXAngleDeg * DEG_TO_RAD) * sinf(orbitYAngleDeg * DEG_TO_RAD);
	drc[1] = sinf(orbitZOXAngleDeg * DEG_TO_RAD);
	drc[2] = cosf(orbitZOXAngleDeg * DEG_TO_RAD) * cosf(orbitYAngleDeg * DEG_TO_RAD);
	//     计算当前相机位置
    static float eye[3];
	eye[0] = center[0] + dist * drc[0];
	eye[1] = center[1] + dist * drc[1];
	eye[2] = center[2] + dist * drc[2];
	static float* view = NULL;
	if (view == NULL)
		view = libg3DGenIdentity4x4(); // 内存漏了但不要紧
	libg3DMat4x4LookAt(view, eye, center);

	//   输出相机位姿
	printf("\n");
	printf("orbitZOXAngleDeg:\t%f\n", orbitZOXAngleDeg);
	printf("OrbitYAngleDeg:\t\t%f\n", orbitYAngleDeg);
	libg3DPrintMat4x4(view, "camera pose");

	libg3DMatInverse(view);
	//   设置透视投影矩阵
	static float* proj = NULL;
	if (proj == NULL)
		proj = libg3DGenPerspective4x4(nearClip, 100.f, FOV, aspectHW);
	else
	{
		free(proj);
		proj = libg3DGenPerspective4x4(nearClip, 100.f, FOV, aspectHW); // 内存漏了但不要紧
	}

	//   输出投影信息
	printf("nearClip:\t%f\n", nearClip);
	printf("FOV:\t\t%f\n", FOV);
	printf("aspectHW:\t%f\n", aspectHW);
	libg3DPrintMat4x4(proj, "projection");

	//   乘起来
	static float* mvp = NULL;
	if (mvp == NULL)
		mvp = libg3DGenIdentity4x4(); // 内存漏了但不要紧
	libg3DMat4x4MulMat4x4(mvp, view, model);
	libg3DPrintMat4x4(mvp, "mv");
	libg3DMat4x4MulMat4x4(mvp, proj, mvp);
	libg3DPrintMat4x4(mvp, "mvp");
	//   在渲染管线中运用当前MVP矩阵
	libg3DApplyMVPMat(&mvp);

	// 绘制
	//   填充视口为灰色
	libg3DClearWithColor(.2f, .2f, .2f);
	//   清除上一次绘制的深度缓冲
	libg3DClearDepthBuffer();
	//   绘制立方体的线框
	libg3DDrawVertsByIndices(LIBG3D_LINES, cubeVerts, cubeIndices, 12 * 2);
}

void Main()
{
	double w = GetFullScreenWidth();
	double h = GetFullScreenHeight();
	SetWindowSize(w, h);
    aspectHW = w / h;

	InitGraphics();
	SetWindowTitle("Hello Wireframe");
	InitConsole();

	// 初始化顶点数据
	float* currVert = cubeVerts;
	for (unsigned char k = 0; k < 8; ++k)
	{
		currVert[0] = (k & 0x1) == 0 ? -.5f : +.5f;
		currVert[1] = (k & 0x2) == 0 ? -.5f : +.5f;
		currVert[2] = (k & 0x4) == 0 ? -.5f : +.5f;
		currVert[3] = (k & 0x1) == 0 ? +0.f : +1.f;
		currVert[4] = (k & 0x2) == 0 ? +0.f : +1.f;
		currVert[5] = (k & 0x4) == 0 ? +0.f : +1.f;
		currVert += 6;
	}

	registerKeyboardEvent(keyboardCallback);
	registerTimerEvent(timerCallback);
	// 15ms，近似于60FPS
	startTimer(0, 15);
}
```

