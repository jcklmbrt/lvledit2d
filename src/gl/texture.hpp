#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <glad/gl.h>
#include <wx/filename.h>

#define THUMB_SIZE_X 32
#define THUMB_SIZE_Y 32
#define MAX_TEXTURE_NAME 32

struct GLTexture
{
	GLuint TextureObject = 0;
	unsigned char *Data;
	size_t PixelWidth;
	size_t Width;
	size_t Height;
	char Name[MAX_TEXTURE_NAME];
	size_t ThumbIndex;
	uint32_t HashValue;
};

void LoadTextureFromMemory(GLTexture *Texture, size_t Width, size_t Height, size_t PixelWidth, const char *Name, unsigned char *Data);
void LoadTextureFromFile(GLTexture *Texture, const wxFileName &FileName);
void DeleteTexture(GLTexture *Texture);
void InitTextureObject(GLTexture *Texture);
bool EqualTextures(GLTexture *A, GLTexture *B);

#endif
