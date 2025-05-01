#ifndef _GLCONTEXT_HPP
#define _GLCONTEXT_HPP

#include <glad/gl.h>
#include <wx/glcanvas.h>

#include "src/geometry.hpp"
#include "src/util/singleton.hpp"
#include "src/gl/texture.hpp"
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
	void Clear(const glm::vec4 &color);
	void SetMatrices(const glm::mat4 &proj, const glm::mat4 &view);
	void DrawElements();
	void AddRect(const glm::vec2 &mins, const glm::vec2 &maxs, const glm::vec4 &color);
	void AddLine(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color);
	void AddPolygon(const ConvexPolygon &poly, const glm::vec4 &color);
	void AddPolygon(const glm::vec2 pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const glm::vec4 &color);
	static GLuint CompileShaders(const char *fs_src, const char *vs_src);
private:
	GLBackgroundGrid m_grid;
	GLSolidGeometry m_solid;
	GLTextureGeometry m_texture;
};

#endif
