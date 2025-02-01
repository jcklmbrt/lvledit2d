#ifndef _GLTEXTUREGEOMETRY_HPP
#define _GLTEXTUREGEOMETRY_HPP

#include <glad/gl.h>
#include <wx/filename.h>
#include "src/geometry.hpp"

struct TextureVertex
{
	Point2D position;
	Color color;
	Point2D uv;
};

#define THUMB_SIZE_X 32
#define THUMB_SIZE_Y 32

#define TEXTURE_SIZE_X 512
#define TEXTURE_SIZE_Y 512

class Texture
{
public:
	Texture(const wxFileName &filename);
	Texture(Texture &&other) noexcept;
	~Texture();
	void Bind();
	void InitTextureObject();
	GLuint GetTextureObject();
	const wxFileName &GetFileName() const { return m_filename; }
	size_t GetIndex() { return m_index; }
	GLuint m_texture = 0;
	unsigned char *m_data;
	size_t m_pixelwidth;
	size_t m_width;
	size_t m_height;
	wxFileName m_filename;
	size_t m_index;
private:
	uint32_t FNV1A();
	uint32_t m_hash;
public:
	bool operator==(const Texture &other);
private:
	Texture(const Texture &) = delete;
	Texture &operator=(const Texture &) = default;
};


class GLTextureGeometry
{
public:
	GLTextureGeometry();
	~GLTextureGeometry();
	void ClearBuffers();
	void CopyBuffersAndDrawElements();
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
public:
	void AddPolygon(const ConvexPolygon &poly, const Color &color);
	void AddPolygon(const Point2D pts[], size_t npts, const Rect2D &uv, Texture &texture, const Color &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	struct TextureVertices
	{
		std::vector<TextureVertex> vtx;
		std::vector<GLuint> idx;
	};

	std::unordered_map<GLuint, TextureVertices> m_batches;
};

#endif