#include <glad/gl.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "src/lvledit2d.hpp"
#include "src/notebook.hpp"
#include "src/glcontext.hpp"
#include "src/glcanvas.hpp"


GLCanvas::GLCanvas(wxWindow *parent, GLContext *context, const wxGLAttributes &attrs)
	: wxGLCanvas(parent, attrs),
	  m_context(context),
	  m_editor(this),
	  m_view(this)
{
	wxSize size = GetSize() * GetContentScaleFactor();
	float width = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);
	m_proj = glm::ortho(0.0f, width, height, 0.0f);

	Bind(wxEVT_SIZE, &GLCanvas::OnSize, this);
	Bind(wxEVT_PAINT, &GLCanvas::OnPaint, this);

	Bind(wxEVT_LEFT_DOWN, &GLCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &GLCanvas::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &GLCanvas::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &GLCanvas::OnMouse, this);
	Bind(wxEVT_MOTION, &GLCanvas::OnMouse, this);

	PushEventHandler(&m_editor);
	PushEventHandler(&m_view);
}


GLCanvas::~GLCanvas()
{
	RemoveEventHandler(&m_view);
	RemoveEventHandler(&m_editor);
}


GLCanvas *GLCanvas::GetCurrent()
{
	GLCanvas *canvas = nullptr;
	MainFrame *mainframe = wxGetApp().GetMainFrame();
	Notebook *notebook = mainframe->GetNotebook();

	if(notebook->GetPageCount() > 0) {
		int sel = notebook->GetSelection();
		wxWindow *page = notebook->GetPage(sel);
		canvas = dynamic_cast<GLCanvas *>(page);
	}

	return canvas;
}


void GLCanvas::OnMouse(wxMouseEvent &e)
{
	m_mousepos = m_view.MouseToWorld(e);
	Refresh(false);
}


void GLCanvas::DrawRect(const Rect2D &rect, const Color &color)
{
	Point2D lt = rect.GetLeftTop();
	Point2D lb = rect.GetLeftBottom();
	Point2D rb = rect.GetRightBottom();
	Point2D rt = rect.GetRightTop();

	float thickness = 1.0 / m_view.GetZoom();
	m_context->AddLine(lt, rt, thickness, color);
	m_context->AddLine(rt, rb, thickness, color);
	m_context->AddLine(rb, lb, thickness, color);
	m_context->AddLine(lb, lt, thickness, color);
}


void GLCanvas::DrawLine(const Point2D &a, const Point2D &b, float thickness, const Color &color)
{
	thickness /= m_view.GetZoom();
	m_context->AddLine(a, b, thickness, color);
}


void GLCanvas::DrawPoint(const Point2D &pt, const Color &color)
{
	const Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	float thickness = 1.0 / m_view.GetZoom();
	Point2D mins = { pt.x - thickness * 3.0f, pt.y - thickness * 3.0f };
	Point2D maxs = { pt.x + thickness * 3.0f, pt.y + thickness * 3.0f };
	Rect2D rect = Rect2D(mins, maxs);

	m_context->AddRect(rect, black); /* outline */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	m_context->AddRect(rect, color); /* foreground */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	m_context->AddRect(rect, black); /* center */
}


void GLCanvas::DrawPolygon(const Point2D points[], size_t npoints, const Color &color)
{
	/* 1 pixel width */
	float thickness = 1.0 / m_view.GetZoom();
	const Color black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	const Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);

	/* background */
	for(size_t i = 0; i < npoints; i++) {
		Point2D a = points[i];
		Point2D b = points[(i + 1) % npoints];
		m_context->AddLine(a, b, thickness * 3.0, black);
	}

	/* foreground */
	for(size_t i = 0; i < npoints; i++) {
		Point2D a = points[i];
		Point2D b = points[(i + 1) % npoints];
		m_context->AddLine(a, b, thickness * 1.0, color);
	}

	/* vertices */
	for(size_t i = 0; i < npoints; i++) {
		DrawPoint(points[i], white);
	}
}


void GLCanvas::DrawPolygon(const ConvexPolygon &poly, const Color &color)
{
	const std::vector<Point2D> &points = poly.GetPoints();
	DrawPolygon(points.data(), points.size(), color);
}


void GLCanvas::OnPaint(wxPaintEvent &e)
{
	m_context->SetCurrent(*this);

	wxPaintDC dc(this);

	Rect2D dummy = {
		Point2D( 100, 100 ),
		Point2D( 200, 200 )
	};	
	
	const Color bg = Color(0.9f, 0.9f, 0.9f, 1.0f);

	m_context->Clear(bg);

	m_editor.OnDraw();

	m_context->CopyBuffers();
	m_context->SetMatrices(m_proj, m_view.GetMatrix());
	m_context->DrawElements();

	m_context->ClearBuffers();

	SwapBuffers();
}


void GLCanvas::OnSize(wxSizeEvent &e)
{
	e.Skip();

	wxSize size = e.GetSize();
	glViewport(0, 0, size.x, size.y);

	float width = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);
	m_proj = glm::ortho(0.0f, width, height, 0.0f);
}