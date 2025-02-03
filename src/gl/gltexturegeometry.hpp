#ifndef _GLTEXTUREGEOMETRY_HPP
#define _GLTEXTUREGEOMETRY_HPP

#include <glad/gl.h>
#include <wx/filename.h>
#include "src/singleton.hpp"
#include "src/geometry.hpp"

struct TextureVertex
{
	Point2D position;
	Color color;
	Point2D uv;
};

struct TextureVertices
{
	std::vector<TextureVertex> vtx;
	std::vector<GLuint> idx;
};

class GLTextureGeometry : Immobile
{
public:
	void Init();
	~GLTextureGeometry();
	void ClearBuffers();
	void CopyBuffersAndDrawElements();
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
public:
	void AddPolygon(const ConvexPolygon &poly, const Color &color);
	void AddPolygon(const Point2D pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const Color &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	std::unordered_map<GLuint, TextureVertices> m_batches;
};

#endif
