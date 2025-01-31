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

class Texture
{
public:
	Texture(const wxFileName &filename);
	~Texture();
	void Bind();
	void InitTextureObject();
	const wxFileName &GetFileName() const { return m_filename; }
	size_t GetIndex() { return m_index; }
	GLuint m_texture = 0;
	unsigned char *m_data;
	size_t m_pixelwidth;
	size_t m_width;
	size_t m_height;
	wxFileName m_filename;
	size_t m_index;
};


class GLTextureGeometry
{
public:
	GLTextureGeometry();
	~GLTextureGeometry();
	void ClearBuffers();
	void CopyBuffers();
	void DrawElements(Texture &texture);
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
public:
	void AddPolygon(const ConvexPolygon &poly, const Color &color);
	void AddPolygon(const Point2D pts[], size_t npts, Texture &texture, const Color &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	std::vector<TextureVertex> m_vtx;
	std::vector<GLuint> m_idx;
};

#endif