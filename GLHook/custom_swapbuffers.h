#pragma once
#include<Windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void CustomSwapBuffers(HDC);
void CustomViewport(int, int, int, int);
void WINAPI OriginalViewport(int, int, int, int);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
DWORD GetDLLPathA(LPSTR path, DWORD max_length);
namespace SpeedGear
{
	BOOL InitCustomTime();
	BOOL UninitCustomTime();
	float GetCurrentSpeed();
}
#ifdef __cplusplus
}
#endif
