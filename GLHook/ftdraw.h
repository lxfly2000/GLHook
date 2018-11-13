#pragma once

#include<glad/glad.h>
#include<map>
#include<glm/glm.hpp>
#include"Shader.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	int Advance;    // Horizontal offset to advance to next glyph
};

class FTDraw
{
private:
	std::map<wchar_t, Character> Characters;
	GLuint ftVAO, ftVBO;
	glm::mat4 projection;
	Shader shader;

public:
	void Init(int width,int height,const char *fontfilename, unsigned int fontsizeh, const wchar_t *usingChars);
	void RenderText(std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
};