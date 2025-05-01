#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <glad/gl.h>
#include <wx/filename.h>

#include "src/edit/l2dfile.hpp"


constexpr int THUMB_SIZE_X = 32;
constexpr int THUMB_SIZE_Y = 32;


class GLTexture
{
public:
	void Load(size_t width, size_t height, size_t pixelwidth, const char *name, unsigned char *data);
	void Load(const wxFileName &filename);
	void Delete();
	void InitTextureObject();
	void AddToFile(L2dTexInfo &info, std::vector<unsigned char> &data, std::vector<unsigned char> &strings) const;
	bool operator==(const GLTexture &other);
private:
	GLuint m_gltex = 0;
	unsigned char *m_data = nullptr;
	size_t m_pixelwidth = 0;
	size_t m_width = 0;
	size_t m_height = 0;
	wxString m_name;
	size_t m_thumb = 0;
	uint32_t m_hash = 0;
public:
	GLuint GetGLObject() const { return m_gltex; }
	const wxString &GetName() const { return m_name; }
	size_t GetThumb() const { return m_thumb; }
	size_t GetWidth() const { return m_width; }
	size_t GetHeight() const { return m_height; }
};

#endif