#pragma once
#include <d2d/d2d_typedef.h>


#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "Msimg32.lib")


KUODAFU_NAMESPACE_BEGIN


typedef DWORD ARGB;

bool d2d_init(bool isMultiThreaded, bool isDebug);
bool d2d_uninit();
D2D_GDI_DATA_STRUCT& d2d_get_info();



KUODAFU_NAMESPACE_END