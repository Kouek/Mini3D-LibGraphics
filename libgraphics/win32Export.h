#ifndef _WIN32_EXPORT_H
#define _WIN32_EXPORT_H

#include <Windows.h>

// 用于调用InvalidateRect()
HWND GetLibgWindow();

// 用于创建HBITMAP
HDC GetLibgOSDC();

// 用于libg3DGetWindowPixelWidth()
int GetLibgWindowPixelWidth();

// 用于libg3DGetWindowPixelHeight()
int GetLibgWindowPixelHeight();

#endif // !_WIN32_EXPORT_H
