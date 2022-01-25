#define FACE
#ifdef FACE
#include <math.h>

#include "libgraphics/extgraph.h"
#include "libg3D/graphics3D.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include <Windows.h>

// �����嶥���� ���Ӧ�� ħ������ͼ
//  v    2---6
// /|\   |   |
//  |0---3---7---10--12
//  ||   |   |   |   |
//  |1---4---8---11--13
//  |    |   |
//  |    5---9
//  o------------------>u   
static float cubeVerts[14 * 8] = {
	// x-----y-----z|-----r------g------b|-----u-------v
	-.5f, +.5f, -.5f, +0.0f, +1.0f, +0.0f, +0.0f, +.666f, // 0
	-.5f, -.5f, -.5f, +0.0f, +0.0f, +0.0f, +0.0f, +.333f,
	-.5f, +.5f, -.5f, +0.0f, +1.0f, +0.0f, +.25f, +1.00f, // 2
	-.5f, +.5f, +.5f, +0.0f, +1.0f, +1.0f, +.25f, +.666f,
	-.5f, -.5f, +.5f, +0.0f, +0.0f, +1.0f, +.25f, +.333f,
	-.5f, -.5f, -.5f, +0.0f, +0.0f, +0.0f, +.25f, +0.00f,
	+.5f, +.5f, -.5f, +1.0f, +1.0f, +0.0f, +.50f, +1.00f, // 6
	+.5f, +.5f, +.5f, +1.0f, +1.0f, +1.0f, +.50f, +.666f,
	+.5f, -.5f, +.5f, +1.0f, +0.0f, +1.0f, +.50f, +.333f,
	+.5f, -.5f, -.5f, +1.0f, +0.0f, +0.0f, +.50f, +0.00f,
	+.5f, +.5f, -.5f, +1.0f, +1.0f, +0.0f, +.75f, +.666f, // 10
	+.5f, -.5f, -.5f, +1.0f, +0.0f, +0.0f, +.75f, +.333f,
	-.5f, +.5f, -.5f, +0.0f, +1.0f, +0.0f, +1.0f, +.666f, // 12
	-.5f, -.5f, -.5f, +0.0f, +0.0f, +0.0f, +1.0f, +.333f,
};
static unsigned int cubeIndices[6 * 2 * 3] = {
	// ��
	1, 4, 3, 3, 0, 1,
	// ǰ
	4, 8, 7, 7, 3, 4,
	// ��
	3, 7, 6, 6, 2, 3,
	// ��
	5, 9, 8, 8, 4, 5,
	// ��
	8, 11, 10, 10, 7, 8,
	// ��
	11, 13, 12, 12, 10, 11
};

static struct Texture* tex = NULL;

// ģ�͡����λ�˲���
static float eyeZOXAngleDeg = 0;
static float eyeYAngleDeg = 0;
static float dist = 2.f;
// ͶӰ����
static float nearClip = .1f;
static float FOV = 90.f;
static float aspectHW = 1.f;

static unsigned char texApplied = 0;
static unsigned char texBilinearFlt = 1;

static unsigned char changed = 1;

static void keyboardCallback(int key, int event)
{
	if (event == KEY_DOWN)
		if (key == VK_RIGHT)
		{
			eyeYAngleDeg += 1.f;
			changed = 1;
		}
		else if (key == VK_LEFT)
		{
			eyeYAngleDeg -= 1.f;
			changed = 1;
		}
		else if (key == VK_UP)
		{
			eyeZOXAngleDeg += 1.f;
			changed = 1;
		}
		else if (key == VK_DOWN)
		{
			eyeZOXAngleDeg -= 1.f;
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
		else if (key == 'T')
		{
			if (texApplied == 0)
			{
				libg3DApplyTexture(&tex);
				texApplied = 1;
			}
			else
			{
				libg3DApplyTexture(NULL);
				texApplied = 0;
			}
			changed = 1;
		}
		else if (key == 'Y')
		{
			if (texBilinearFlt == 0)
			{
				libg3DSetTextureMagFilterMode(LIBG3D_TEXFILT_LINEAR);
				libg3DSetTextureMinFilterMode(LIBG3D_TEXFILT_LINEAR);
				texBilinearFlt = 1;
			}
			else
			{
				libg3DSetTextureMagFilterMode(LIBG3D_TEXFILT_NEAREST);
				libg3DSetTextureMinFilterMode(LIBG3D_TEXFILT_NEAREST);
				texBilinearFlt = 0;
			}
			changed = 1;
		}
}

static void timerCallback(int timerID)
{
	// ������֡������
	if (changed == 0)return;
	changed = 0;

	// �����ӿ�
	unsigned int pixW = libg3DGetWindowPixelWidth();
	unsigned int pixH = libg3DGetWindowPixelHeight();
	libg3DViewport(pixW / 8, pixH / 8, pixW * 3 / 4, pixH * 3 / 4);

	// ����MVP
	//  ����ģ�;���Ϊ��λ��
	static float* model = NULL;
	if (model == NULL)
		model = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	//   ����һ���������
	//     ��ʼ���������ĵ�Ϊԭ��
	static float center[3] = { 0,0,0 };
	//     ��ʼ��ԭ��ָ������ķ���Ϊ x��������
	static float drc[3] = { 1.f, 0, 0 };
	//     ���㵱ǰ����
	drc[0] = cosf(eyeZOXAngleDeg * DEG_TO_RAD) * sinf(eyeYAngleDeg * DEG_TO_RAD);
	drc[1] = sinf(eyeZOXAngleDeg * DEG_TO_RAD);
	drc[2] = cosf(eyeZOXAngleDeg * DEG_TO_RAD) * cosf(eyeYAngleDeg * DEG_TO_RAD);
	static float eye[3];
	//     ���㵱ǰ���λ��
	eye[0] = center[0] + dist * drc[0];
	eye[1] = center[1] + dist * drc[1];
	eye[2] = center[2] + dist * drc[2];
	static float* view = NULL;
	if (view == NULL)
		view = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	libg3DMat4x4LookAt(view, eye, center);

	//   ������λ��
	printf("\n");
	printf("eyeYAngleDeg:%20f\n", eyeYAngleDeg);
	printf("eyeZOXAngleDeg:%20f\n", eyeZOXAngleDeg);
	libg3DPrintMat4x4(view, "camera pose");

	libg3DMatInverse(view);
	//   ����͸��ͶӰ����
	static float* proj = NULL;
	if (proj == NULL)
		proj = libg3DGenPerspective4x4(nearClip, 100.f, FOV, aspectHW);
	else
		libg3DSetPerspective4x4(proj, nearClip, 100.f, FOV, aspectHW);

	//   ���ͶӰ��Ϣ
	printf("nearClip:\t%f\n", nearClip);
	printf("FOV:\t\t%f\n", FOV);
	printf("aspectHW:\t%f\n", aspectHW);
	libg3DPrintMat4x4(proj, "projection");

	//   ������
	static float* mvp = NULL;
	if (mvp == NULL)
		mvp = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	libg3DMat4x4MulMat4x4(mvp, view, model);
	libg3DPrintMat4x4(mvp, "mv");
	libg3DMat4x4MulMat4x4(mvp, proj, mvp);
	libg3DPrintMat4x4(mvp, "mvp");
	//   ����Ⱦ���������õ�ǰMVP����
	libg3DApplyMVPMat(&mvp);

	// ����
	//   ����ӿ�Ϊ��ɫ
	libg3DClearWithColor(.2f, .2f, .2f);
	//   �����һ�λ��Ƶ���Ȼ���
	libg3DClearDepthBuffer();
	//   ����������
	libg3DDrawVertsByIndices(LIBG3D_TRIANGLES, cubeVerts, cubeIndices, 6 * 2 * 3);
}

void Main()
{
	double w = 500.0;
	double h = 500.0;
	SetWindowSize(w, h);
	aspectHW = w / h;

	InitGraphics();
	SetWindowTitle("Hello Wireframe");
	InitConsole();

	// ��������
	int tw, th, channels;
	stbi_set_flip_vertically_on_load(1); // ��ת�����ͼƬʹ��y�ᳯ��
	unsigned char* imgDat = stbi_load("./rsc/cube.png", &tw, &th, &channels, 0);
	tex = libg3DGenTexture(imgDat, tw, th, LIBG3D_TEXFORM_RGBA);
	stbi_image_free(imgDat);

	libg3DSetVertAttributes(3, 3, 2);

	registerKeyboardEvent(keyboardCallback);
	registerTimerEvent(timerCallback);
	// 33ms��������30FPS
	startTimer(0, 33);
}

#else
#include <math.h>

#include "libgraphics/extgraph.h"
#include "libg3D/graphics3D.h"

#include <Windows.h>

// �߿������嶥����
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
	// ����
	0, 1, 1, 3, 3, 2, 2, 0,
	// ǰ��
	4, 5, 5, 7, 7, 6, 6, 4,
	// ����
	0, 4, 2, 6, 1, 5, 3, 7
};

// ģ�͡����λ�˲���
static float eyeZOXAngleDeg = 0;
static float eyeYAngleDeg = 0;
static float dist = 2.f;
// ͶӰ����
static float nearClip = .1f;
static float FOV = 90.f;
static float aspectHW = 1.f;

static unsigned char changed = 1;

static void keyboardCallback(int key, int event)
{
	if (event == KEY_DOWN)
		if (key == VK_RIGHT)
		{
			eyeYAngleDeg += 1.f;
			changed = 1;
		}
		else if (key == VK_LEFT)
		{
			eyeYAngleDeg -= 1.f;
			changed = 1;
		}
		else if (key == VK_UP)
		{
			eyeZOXAngleDeg += 1.f;
			changed = 1;
		}
		else if (key == VK_DOWN)
		{
			eyeZOXAngleDeg -= 1.f;
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
	// ������֡������
	if (changed == 0)return;
	changed = 0;

	// �����ӿ�
	unsigned int pixW = libg3DGetWindowPixelWidth();
	unsigned int pixH = libg3DGetWindowPixelHeight();
	libg3DViewport(pixW / 8, pixH / 8, pixW * 3 / 4, pixH * 3 / 4);

	// ÿ��������6��float�������ԣ�ǰ3Ϊλ����������3Ϊ��ɫ����
	libg3DSetVertAttributes(3, 3, 0);

	// ����MVP
	//  ����ģ�;���Ϊ��λ��
	static float* model = NULL;
	if (model == NULL)
		model = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	//   ����һ���������
	//     ��ʼ���������ĵ�Ϊԭ��
	static float center[3] = { 0,0,0 };
	//     ��ʼ��ԭ��ָ������ķ���Ϊ x��������
	static float drc[3] = { 1.f, 0, 0 };
	//     ���㵱ǰ����
	drc[0] = cosf(eyeZOXAngleDeg * DEG_TO_RAD) * sinf(eyeYAngleDeg * DEG_TO_RAD);
	drc[1] = sinf(eyeZOXAngleDeg * DEG_TO_RAD);
	drc[2] = cosf(eyeZOXAngleDeg * DEG_TO_RAD) * cosf(eyeYAngleDeg * DEG_TO_RAD);
	static float eye[3];
	//     ���㵱ǰ���λ��
	eye[0] = center[0] + dist * drc[0];
	eye[1] = center[1] + dist * drc[1];
	eye[2] = center[2] + dist * drc[2];
	static float* view = NULL;
	if (view == NULL)
		view = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	libg3DMat4x4LookAt(view, eye, center);

	//   ������λ��
	printf("\n");
	printf("eyeYAngleDeg:%20f\n", eyeYAngleDeg);
	printf("eyeZOXAngleDeg:%20f\n", eyeZOXAngleDeg);
	libg3DPrintMat4x4(view, "camera pose");

	libg3DMatInverse(view);
	//   ����͸��ͶӰ����
	static float* proj = NULL;
	if (proj == NULL)
		proj = libg3DGenPerspective4x4(nearClip, 100.f, FOV, aspectHW);
	else
		libg3DSetPerspective4x4(proj, nearClip, 100.f, FOV, aspectHW);

	//   ���ͶӰ��Ϣ
	printf("nearClip:\t%f\n", nearClip);
	printf("FOV:\t\t%f\n", FOV);
	printf("aspectHW:\t%f\n", aspectHW);
	libg3DPrintMat4x4(proj, "projection");

	//   ������
	static float* mvp = NULL;
	if (mvp == NULL)
		mvp = libg3DGenIdentity4x4(); // �ڴ�©�˵���Ҫ��
	libg3DMat4x4MulMat4x4(mvp, view, model);
	libg3DPrintMat4x4(mvp, "mv");
	libg3DMat4x4MulMat4x4(mvp, proj, mvp);
	libg3DPrintMat4x4(mvp, "mvp");
	//   ����Ⱦ���������õ�ǰMVP����
	libg3DApplyMVPMat(&mvp);

	// ����
	//   ����ӿ�Ϊ��ɫ
	libg3DClearWithColor(.2f, .2f, .2f);
	//   �����һ�λ��Ƶ���Ȼ���
	libg3DClearDepthBuffer();
	//   ������������߿�
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

	// ��ʼ����������
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
	// 22ms��������45FPS
	startTimer(0, 22);
}
#endif