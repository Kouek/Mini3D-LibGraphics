#ifndef _WIN32_EXPORT_H
#define _WIN32_EXPORT_H

#include <Windows.h>

HWND GetLibgWindow();

HDC GetLibgOSDC();

int GetLibgWindowPixelWidth();

int GetLibgWindowPixelHeight();

#endif // !_WIN32_EXPORT_H
