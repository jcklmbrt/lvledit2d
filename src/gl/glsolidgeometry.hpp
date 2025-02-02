#ifndef _GLSOLIDGEOMETRY_HPP
#define _GLSOLIDGEOMETRY_HPP

#include <glad/gl.h>
#include "src/singleton.hpp"
#include "src/geometry.hpp"

struct SolidVertex
{
	Point2D position;
	Color color;
};

class GLSolidGeometry : Immobile
{
public:
	void Init();
	~GLSolidGeometry();
	void ClearBuffers();
	void CopyBuffers();
	void DrawElements();
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
	void AddRect(const Rect2D &rect, const Color &color);
	void AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
private:
	void AddQuad(const Point2D q[4], const Color &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	std::vector<SolidVertex> m_vtx;
	std::vector<GLuint> m_idx;
};

#endif
