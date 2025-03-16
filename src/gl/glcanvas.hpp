#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <glad/gl.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/edit/editorcontext.hpp"
#include "src/viewmatrix.hpp"

class Notebook;
class GLContext;
class GLTexture;
class ConvexPolygon;

class GLCanvas : public wxGLCanvas
{
public:
	GLCanvas(Notebook *parent, const wxGLAttributes &attrs);
	virtual ~GLCanvas();
	void DrawPoint(const glm::vec2 &point, const glm::vec4 &color);
	void OutlineRect(const Rect2D &rect, float thickness, const glm::vec4 &color);
	void OutlinePoly(const glm::vec2 points[], size_t npoints, float thickness, const glm::vec4 &color);
	void TexturePoly(const ConvexPolygon &p, const glm::vec4 &color);
	void TexturePoly(const glm::vec2 pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const glm::vec4 &color);
	void DrawLine(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color);
	static GLCanvas *GetCurrent();
	void OnPaint(wxPaintEvent &e);
	void OnSize(wxSizeEvent &e);
	void OnMouse(wxMouseEvent &e);
private:
	glm::vec2 m_mousepos;
	EditorContext m_editor;
	ViewMatrixCtrl m_view;
	glm::mat4 m_proj;
public:
	inline glm::vec2 GetMousePos() { return m_mousepos; }
	inline EditorContext &GetEditor() { return m_editor; }
	inline glm::mat4 &GetViewMatrix() { return m_view.GetMatrix(); }
	inline glm::mat4 &GetProjMatrix() { return m_proj; }
	inline ViewMatrixBase &GetView() { return m_view; }
};


#endif
