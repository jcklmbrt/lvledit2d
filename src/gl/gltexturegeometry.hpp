#ifndef _GLTEXTUREGEOMETRY_HPP
#define _GLTEXTUREGEOMETRY_HPP

#include <glad/gl.h>
#include <wx/filename.h>
#include "src/singleton.hpp"
#include "src/geometry.hpp"

struct TextureVertex
{
	glm::vec2 position;
	glm::vec4 color;
	glm::vec2 uv;
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
	void SetMatrices(const glm::mat4 &proj, const glm::mat4 &view);
public:
	void AddPolygon(const ConvexPolygon &poly, const glm::vec4 &color);
	void AddPolygon(const glm::vec2 pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const glm::vec4 &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	std::unordered_map<GLuint, TextureVertices> m_batches;
};

#endif
