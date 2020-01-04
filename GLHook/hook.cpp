//https://pastebin.com/f6d87dd03
#include<Windows.h>
#include"glad\glad.h"
#include"..\minhook\include\MinHook.h"

#include"custom_swapbuffers.h"

typedef BOOL(WINAPI*PFwglSwapBuffers)(HDC);
typedef PFNGLVIEWPORTPROC PFglViewport;
static PFwglSwapBuffers pfSwapBuffers = nullptr, pfOriginalSwapBuffers = nullptr;
static PFglViewport pfViewport = nullptr, pfOriginalViewport = nullptr;
static HMODULE hDllModule;

DWORD GetDLLPath(LPTSTR path, DWORD max_length)
{
	return GetModuleFileName(hDllModule, path, max_length);
}

DWORD GetDLLPathA(LPSTR path, DWORD max_length)
{
	return GetModuleFileNameA(hDllModule, path, max_length);
}

BOOL WINAPI HookedwglSwapBuffers(HDC p)
{
	CustomSwapBuffers(p);
	return pfOriginalSwapBuffers(p);
}

void WINAPI HookedglViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	CustomViewport(x, y, width, height);
	return pfOriginalViewport(x, y, width, height);
}

void WINAPI OriginalViewport(int x, int y, int width, int height)
{
	return pfOriginalViewport(x, y, width, height);
}

PFwglSwapBuffers GetSwapBuffersAddr()
{
	return reinterpret_cast<PFwglSwapBuffers>(GetProcAddress(LoadLibrary(TEXT("OpenGL32.dll")), "wglSwapBuffers"));
}

PFglViewport GetViewportAddr()
{
	return reinterpret_cast<PFglViewport>(GetProcAddress(LoadLibrary(TEXT("OpenGL32.dll")), "glViewport"));
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	pfSwapBuffers = GetSwapBuffersAddr();
	pfViewport = GetViewportAddr();
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfSwapBuffers, HookedwglSwapBuffers, reinterpret_cast<void**>(&pfOriginalSwapBuffers)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfViewport, HookedglViewport, reinterpret_cast<void**>(&pfOriginalViewport)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfViewport) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfViewport) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfViewport) != MH_OK)
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
	hDllModule = hInstDll;
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