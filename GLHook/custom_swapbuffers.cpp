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
	TCHAR text[128];
	unsigned t1, t2, fcounter;
	RECT windowrect;
	HDC m_hdc;
	FTDraw ftdraw;
	const int fontsize = 48;
public:
	SwapBuffersDraw():t1(0),t2(0),fcounter(0),m_hdc(NULL)
	{
	}
	void Init(HDC dc)
	{
		m_hdc = dc;
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
		//https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_opengl3.cpp#L142
#pragma region Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#ifdef GL_POLYGON_MODE
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
		bool clip_origin_lower_left = true;
#ifdef GL_CLIP_ORIGIN
		GLenum last_clip_origin; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		clip_origin_lower_left = (last_clip_origin == GL_LOWER_LEFT);
#endif
#pragma endregion
		//Notice that the origin point is at bottom-left of the screen
		ftdraw.RenderText(text, 2.0f, windowrect.bottom - windowrect.top - fontsize - 2.0f, 1.0f, glm::vec3(0.5f, 0.5f, 0.5f));
		ftdraw.RenderText(text, 0, (float)(windowrect.bottom-windowrect.top-fontsize), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
#pragma region Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, last_sampler);
#endif
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
#pragma endregion
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
