//https://pastebin.com/f6d87dd03
#include<Windows.h>
#include"..\minhook\include\MinHook.h"

#include"custom_swapbuffers.h"

typedef BOOL(WINAPI*PFwglSwapBuffers)(HDC);
static PFwglSwapBuffers pfSwapBuffers = nullptr, pfOriginalSwapBuffers = nullptr;

BOOL WINAPI HookedwglSwapBuffers(HDC p)
{
	CustomSwapBuffers(p);
	return pfOriginalSwapBuffers(p);
}

PFwglSwapBuffers GetSwapBuffersAddr()
{
	return reinterpret_cast<PFwglSwapBuffers>(GetProcAddress(LoadLibrary(TEXT("OpenGL32.dll")), "wglSwapBuffers"));
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	pfSwapBuffers = GetSwapBuffersAddr();
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfSwapBuffers, HookedwglSwapBuffers, reinterpret_cast<void**>(&pfOriginalSwapBuffers)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_Uninitialize() != MH_OK)
		return FALSE;
	return TRUE;
}

DWORD WINAPI TInitHook(LPVOID param)
{
	return StartHook();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDll);
		CreateThread(NULL, 0, TInitHook, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	}
	return TRUE;
}

//SetWindowHookEx需要一个导出函数，否则DLL不会被加载
extern "C" __declspec(dllexport) LRESULT WINAPI HookProc(int code, WPARAM w, LPARAM l)
{
	return CallNextHookEx(NULL, code, w, l);
}