#ifndef _GLCANVAS_HPP
#define _GLCANVAS_HPP

#include <glad/gl.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/edit/editorcontext.hpp"
#include "src/glcontext.hpp"
#include "src/viewmatrix.hpp"

class GLContext;
class GLCanvas;

class GLCanvas : public wxGLCanvas
{
public:
	GLCanvas(wxWindow *parent, GLContext *context, const wxGLAttributes &attrs);
	virtual ~GLCanvas();
	void DrawPolygon(const ConvexPolygon &poly, const Color &color);
	void DrawPolygon(const Point2D points[], size_t npoints, const Color &color);
	void DrawPoint(const Point2D &point, const Color &color);
	void DrawRect(const Rect2D &rect, const Color &color);
	void DrawLine(const Point2D &a, const Point2D &b, float thickness, const Color &color);
public:
	int  GetGridSpacing() { return 50; }
	bool IsSnapToGrid() { return m_editor.IsSnapToGrid(); }
	Point2D GetMousePos() { return m_mousepos; }
	EditorContext &GetEditor() { return m_editor; };
	ViewMatrix &GetView() { return m_view; }

	static GLCanvas *GetCurrent();
private:
	void OnPaint(wxPaintEvent &e);
	void OnSize(wxSizeEvent &e);

	void OnMouse(wxMouseEvent &e);
	/* cache mouse position for use in draw routines */
	Point2D m_mousepos;

	GLContext *m_context;
	ViewMatrixCtrl m_view;
	EditorContext m_editor;
	Matrix4 m_proj;
};


#endif