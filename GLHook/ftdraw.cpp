#include "ftdraw.h"

#include<ft2build.h>
#include FT_FREETYPE_H
#include<algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma comment(lib,"../freetype-2.9.1/win64/freetype.lib")

const char vshaderCode[] = "#version 330 core\n\
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n\
out vec2 TexCoords;\n\
\n\
uniform mat4 projection;\n\
\n\
void main()\n\
{\n\
	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n\
	TexCoords = vertex.zw;\n\
}";

const char fshaderCode[] = "#version 330 core\n\
in vec2 TexCoords;\n\
out vec4 color;\n\
\n\
uniform sampler2D text;\n\
uniform vec3 textColor;\n\
\n\
void main()\n\
{\n\
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n\
	color = vec4(textColor, 1.0) * sampled;\n\
}";

void FTDraw::Init(int width,int height,const char *fontfilename, unsigned int fontsizeh, const wchar_t *usingChars)
{
	int success=gladLoadGL();
	assert(success);
	shader.Init(vshaderCode, fshaderCode);
	projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &ftVAO);
	glGenBuffers(1, &ftVBO);
	glBindVertexArray(ftVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ftVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// FreeType
	FT_Library ft;
	int error = 0;
	// All functions return a value different than 0 whenever an error occurred
	error = FT_Init_FreeType(&ft);
	assert(error == 0);

	// Load font as face
	FT_Face face;
	error = FT_New_Face(ft, fontfilename, 0, &face);
	assert(error == 0);
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, fontsizeh);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	std::wstring puc;
	for (wchar_t c = ' '; c < 128; c++)
		puc.append(1, c);
	if (usingChars)
	{
		puc.append(usingChars);
		std::sort(puc.begin(), puc.end());
		puc.erase(std::unique(puc.begin(), puc.end()), puc.end());
	}
	for (size_t i = 0; puc[i]; i++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, puc[i], FT_LOAD_RENDER))
		{
			assert(0);
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::make_pair(puc[i], character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


}

void FTDraw::RenderText(std::wstring text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(ftVAO);

	// Iterate through all characters
	std::wstring::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, ftVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

