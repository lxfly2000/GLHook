#pragma once
#include<Windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void CustomSwapBuffers(HDC);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
DWORD GetDLLPathA(LPSTR path, DWORD max_length);
#ifdef __cplusplus
}
#endif
