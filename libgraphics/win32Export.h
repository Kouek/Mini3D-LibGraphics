#ifndef _WIN32_EXPORT_H
#define _WIN32_EXPORT_H

#include <Windows.h>

// ���ڵ���InvalidateRect()
HWND GetLibgWindow();

// ���ڴ���HBITMAP
HDC GetLibgOSDC();

// ����libg3DGetWindowPixelWidth()
int GetLibgWindowPixelWidth();

// ����libg3DGetWindowPixelHeight()
int GetLibgWindowPixelHeight();

#endif // !_WIN32_EXPORT_H
