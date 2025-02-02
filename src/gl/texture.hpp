#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <glad/gl.h>
#include <wx/filename.h>

#define THUMB_SIZE_X 32
#define THUMB_SIZE_Y 32

class Texture
{
public:
	Texture(const wxFileName &filename);
	Texture(Texture &&other) noexcept;
	~Texture();
	void InitTextureObject();
	GLuint GetTextureObject();
	const wxFileName &GetFileName() const { return m_filename; }
	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
	size_t GetIndex() const { return m_index; }
	bool operator==(const Texture &other);
private:
	uint32_t FNV1A();
	uint32_t m_hash;

	/* disable copying */
	Texture(const Texture &) = delete;
	Texture &operator=(const Texture &) = default;

	GLuint m_texture = 0;
	unsigned char *m_data;
	size_t m_pixelwidth;
	size_t m_width;
	size_t m_height;
	wxFileName m_filename;
	size_t m_index;
};


#endif