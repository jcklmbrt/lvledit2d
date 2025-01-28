#ifndef _GLCONTEXT_HPP
#define _GLCONTEXT_HPP

#include "glad/gl.h"
#include "src/geometry.hpp"
#include <wx/glcanvas.h>

class GLSolidGeometry;
class GLBackgroundGrid;

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
	void AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
	static GLuint CompileShaders(const char *fs_src, const char *vs_src);
private:
	GLBackgroundGrid *m_grid;
	GLSolidGeometry *m_solid;
};

#endif
