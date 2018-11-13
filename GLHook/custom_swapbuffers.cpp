#include "custom_swapbuffers.h"

//https://stackoverflow.com/a/13438807
BOOL CheckWindowsVersion(DWORD dwMajor, DWORD dwMinor, DWORD dwBuild)
{
	// Initialize the OSVERSIONINFOEX structure.
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = dwMajor;
	osvi.dwMinorVersion = dwMinor;
	osvi.dwBuildNumber = dwBuild;

	// Initialize the condition mask.
	DWORDLONG dwlConditionMask = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	// Perform the test.
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
}

class SwapBuffersDraw
{
private:
	HWND hWnd;
	NOTIFYICONDATA nid;
public:
	HDC hDC;
	SwapBuffersDraw():hDC(NULL),hWnd(NULL)
	{
		nid.hWnd = NULL;
	}
	~SwapBuffersDraw()
	{
		if (nid.hWnd)
			Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	void Init(HDC dc)
	{
		if (nid.hWnd)
			Shell_NotifyIcon(NIM_DELETE, &nid);
		hDC = dc;
		hWnd = WindowFromDC(hDC);
		if (CheckWindowsVersion(6, 0, 6))
			nid.cbSize = sizeof(NOTIFYICONDATA);
		else if (CheckWindowsVersion(6, 0, 0))
			nid.cbSize = NOTIFYICONDATA_V3_SIZE;
		else if (CheckWindowsVersion(5, 0, 0))
			nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		else
			nid.cbSize = NOTIFYICONDATA_V1_SIZE;
		nid.hWnd = hWnd;
		nid.uID = 2000;
		nid.hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
		nid.uVersion = NOTIFYICON_VERSION;
		nid.uTimeout = 5000;
		TCHAR text[256];
		GetWindowText(hWnd, text, ARRAYSIZE(text));
		wsprintf(nid.szTip, TEXT("GLHook [%s]"), text);
		lstrcpy(nid.szInfoTitle, TEXT("GLHook º”‘ÿ≥…π¶°£"));
		wsprintf(nid.szInfo, TEXT("HDC: %p\nHWND: %p"), hDC, hWnd);
		nid.dwInfoFlags = NIIF_INFO;
		Shell_NotifyIcon(NIM_ADD, &nid);
	}

	void Draw()
	{
		static TCHAR title[256];
		static unsigned t1 = 0, t2 = 0, fcounter = 0;
		if (fcounter-- == 0)
		{
			fcounter = 60;
			t1 = t2;
			t2 = GetTickCount();
			wsprintf(title, TEXT("FPS: %d"), 60000 / (t2 - t1));
			SetWindowText(hWnd, title);
		}
	}
};

static SwapBuffersDraw c;

void CustomSwapBuffers(HDC pDC)
{
	if (c.hDC != pDC)
		c.Init(pDC);
	c.Draw();
}
