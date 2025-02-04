#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <glad/gl.h>
#include <wx/filename.h>

#define THUMB_SIZE_X 32
#define THUMB_SIZE_Y 32
#define MAX_TEXTURE_NAME 32

struct GLTexture
{
	void Load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data);
	void Load(const wxFileName &filename);
	void Delete();
	void InitTextureObject();

	bool operator==(const GLTexture &other);

	GLuint gltex = 0;
	unsigned char *data;
	size_t pixelwidth;
	size_t width;
	size_t height;
	char name[MAX_TEXTURE_NAME];
	size_t thumb;
	uint32_t hash;
};

#endif
