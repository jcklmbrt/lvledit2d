#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <glad/gl.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/edit/editorcontext.hpp"
#include "src/viewmatrix.hpp"

class Notebook;
class GLContext;
struct ConvexPolygon;

struct GLCanvas : public wxGLCanvas
{
	GLCanvas(Notebook *parent, const wxGLAttributes &attrs);
	virtual ~GLCanvas();
	void DrawPoint(const Point2D &point, const Color &color);
	void OutlineRect(const Rect2D &rect, float thickness, const Color &color);
	void OutlinePoly(const Point2D points[], size_t npoints, float thickness, const Color &color);
	void TexturePoly(const ConvexPolygon &p, const Color &color);
	void TexturePoly(const Point2D pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const Color &color);
	void DrawLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
	static GLCanvas *GetCurrent();
	void OnPaint(wxPaintEvent &e);
	void OnSize(wxSizeEvent &e);

	void OnMouse(wxMouseEvent &e);
	/* cache mouse position for use in draw routines */
	Point2D mousepos;
	EditorContext editor;

	ViewMatrixCtrl view;
	Matrix4 proj;
};


#endif
