#include "custom_swapbuffers.h"
#include"ftdraw.h"
#include<map>
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")

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
	NOTIFYICONDATA nid;
	TCHAR text[128];
	unsigned t1, t2, fcounter;
	RECT windowrect;
	HDC m_hdc;
	FTDraw ftdraw;
	const int fontsize = 48;
public:
	SwapBuffersDraw():t1(0),t2(0),fcounter(0),m_hdc(NULL)
	{
		nid.hWnd = NULL;
	}
	~SwapBuffersDraw()
	{
		if (nid.hWnd)
			Shell_NotifyIcon(NIM_DELETE, &nid);
	}
	void ShowTrayBalloon(HDC hDC)
	{
		if (nid.hWnd)
			Shell_NotifyIcon(NIM_DELETE, &nid);
		nid.hWnd = WindowFromDC(hDC);
		if (CheckWindowsVersion(6, 0, 6))
			nid.cbSize = sizeof(NOTIFYICONDATA);
		else if (CheckWindowsVersion(6, 0, 0))
			nid.cbSize = NOTIFYICONDATA_V3_SIZE;
		else if (CheckWindowsVersion(5, 0, 0))
			nid.cbSize = NOTIFYICONDATA_V2_SIZE;
		else
			nid.cbSize = NOTIFYICONDATA_V1_SIZE;
		nid.uID = 2000;
		nid.hIcon = (HICON)GetClassLongPtr(nid.hWnd, GCLP_HICON);
		nid.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
		nid.uVersion = NOTIFYICON_VERSION;
		nid.uTimeout = 5000;
		TCHAR text[256];
		GetWindowText(nid.hWnd, text, ARRAYSIZE(text));
		wsprintf(nid.szTip, TEXT("GLHook [%s]"), text);
		lstrcpy(nid.szInfoTitle, TEXT("GLHook ���سɹ���"));
		wsprintf(nid.szInfo, TEXT("HDC: %p\nHWND: %p"), hDC, nid.hWnd);
		nid.dwInfoFlags = NIIF_INFO;
		Shell_NotifyIcon(NIM_ADD, &nid);
	}
	void Init(HDC dc)
	{
		m_hdc = dc;
		ShowTrayBalloon(dc);
		GetClientRect(WindowFromDC(dc), &windowrect);
		char fontname[256];
		size_t rlen;
		getenv_s(&rlen, fontname, "windir");
		strcat_s(fontname, "/Fonts/SimSun.ttc");
		ftdraw.Init(windowrect.right - windowrect.left, windowrect.bottom - windowrect.top, "C:/Windows/Fonts/simsun.ttc", fontsize, NULL);
	}

	void Draw()
	{
		if (fcounter-- == 0)
		{
			fcounter = 60;
			t1 = t2;
			t2 = GetTickCount();
			if (t1 == t2)
				t1--;
			wsprintf(text, TEXT("FPS: %d"), 60000 / (t2 - t1));
		}
		ftdraw.RenderText(text, 0, (float)(windowrect.bottom-windowrect.top-fontsize), 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
	}
};

static std::map<HDC, SwapBuffersDraw>cp;

void CustomSwapBuffers(HDC pDC)
{
	if (cp.find(pDC) == cp.end())
	{
		cp.insert(std::make_pair(pDC, SwapBuffersDraw()));
		cp[pDC].Init(pDC);
	}
	cp[pDC].Draw();
}
