#ifndef _GLCONTEXT_HPP
#define _GLCONTEXT_HPP

#include "src/geometry.hpp"
#include <wx/glcanvas.h>


class GLContext;

class GLBackgroundGrid
{
public:
	GLBackgroundGrid(GLContext *context);
	~GLBackgroundGrid();
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
	void DrawGrid();
	constexpr static int SPACING = 50;
	static void Snap(Point2D &pt)
	{
		pt.x = round(pt.x / SPACING) * SPACING;
		pt.y = round(pt.y / SPACING) * SPACING;
	}
private:
	GLContext *m_context;
	GLuint m_program;
	GLuint m_vtxbuf;
	GLuint m_vao;
};


class GLContext : public wxGLContext
{
public:
	GLContext(wxGLCanvas *parent);
	virtual ~GLContext();
	void ClearBuffers();
	void CopyBuffers();
	void Clear(const Color &color);
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
	void DrawElements();
	void AddRect(const Rect2D &rect, const Color &color);
	void AddQuad(const Point2D q[4], const Color &color);
	void AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
private:
	GLuint m_vao;
	GLuint m_program;

	GLuint m_vtxbuf;
	std::vector<Point2D> m_points;

	GLuint m_colbuf;
	std::vector<Color> m_colors;

	GLuint m_idxbuf;
	std::vector<GLuint> m_indices;

	GLBackgroundGrid *m_grid;
};

#endif
