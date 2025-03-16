#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <glad/gl.h>
#include <wx/filename.h>

#define THUMB_SIZE_X 32
#define THUMB_SIZE_Y 32
#define MAX_TEXTURE_NAME 32

/* stripped down version of GLTexture */
struct TexInfo 
{
	char name[MAX_TEXTURE_NAME];
	uint32_t width;
	uint32_t height;
	uint8_t pixelwidth;
	uint32_t dataoffset;
};

class GLTexture
{
public:
	void Load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data);
	void Load(const wxFileName &filename);
	void Delete();
	void InitTextureObject();
	void AddToFile(TexInfo &info, std::vector<unsigned char> &data);
	bool operator==(const GLTexture &other);
private:
	GLuint m_gltex = 0;
	unsigned char *m_data = nullptr;
	size_t m_pixelwidth = 0;
	size_t m_width = 0;
	size_t m_height = 0;
	char m_name[MAX_TEXTURE_NAME] = {};
	size_t m_thumb = 0;
	uint32_t m_hash = 0;
public:
	GLuint GetGLObject() const { return m_gltex; }
	const char *GetName() const { return m_name; }
	size_t GetThumb() const { return m_thumb; }
	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
};

#endif