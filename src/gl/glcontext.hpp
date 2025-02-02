#ifndef _GLCONTEXT_HPP
#define _GLCONTEXT_HPP

#include <glad/gl.h>
#include <wx/glcanvas.h>

#include "src/geometry.hpp"
#include "src/singleton.hpp"
#include "src/gl/glsolidgeometry.hpp"
#include "src/gl/gltexturegeometry.hpp"
#include "src/gl/glbackgroundgrid.hpp"

class wxFileName;

class GLContext : public wxGLContext,
                  public Singleton<GLContext>
{
public:
	GLContext(wxGLCanvas *parent);
	void ClearBuffers();
	void CopyBuffers();
	void Clear(const Color &color);
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
	void DrawElements();
	void AddRect(const Rect2D &rect, const Color &color);
	void AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
	void AddPolygon(const ConvexPolygon &poly, const Color &color);
	void AddPolygon(const Point2D pts[], size_t npts, const Rect2D &uv, Texture &texture, const Color &color);
	static GLuint CompileShaders(const char *fs_src, const char *vs_src);
private:
	GLBackgroundGrid m_grid;
	GLSolidGeometry m_solid;
	GLTextureGeometry m_texture;
};

#endif
