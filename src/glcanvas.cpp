#include <cassert>
#include <glad/gl.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "src/lvledit2d.hpp"
#include "src/notebook.hpp"
#include "src/glcontext.hpp"
#include "src/glcanvas.hpp"


GLCanvas::GLCanvas(Notebook *parent, const wxGLAttributes &attrs)
	: wxGLCanvas(parent, attrs),
	  m_editor(this),
	  m_view(this)
{
	assert(wxGLCanvas::IsDisplaySupported(attrs));

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


void GLCanvas::OutlineRect(const Rect2D &rect, float thickness, const Color &color)
{
	Point2D lt = rect.GetLeftTop();
	Point2D lb = rect.GetLeftBottom();
	Point2D rb = rect.GetRightBottom();
	Point2D rt = rect.GetRightTop();

	thickness /= m_view.GetZoom();
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
	float thickness = 1.0 / m_view.GetZoom();
	Point2D mins = { pt.x - thickness * 3.0f, pt.y - thickness * 3.0f };
	Point2D maxs = { pt.x + thickness * 3.0f, pt.y + thickness * 3.0f };
	Rect2D rect = Rect2D(mins, maxs);

	m_context->AddRect(rect, BLACK); /* outline */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	m_context->AddRect(rect, color); /* foreground */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	m_context->AddRect(rect, BLACK); /* center */
}


void GLCanvas::OutlinePoly(const Point2D points[], size_t npoints, float thickness, const Color &color)
{
	thickness /= m_view.GetZoom();

	for(size_t i = 0; i < npoints; i++) {
		Point2D a = points[i];
		Point2D b = points[(i + 1) % npoints];
		m_context->AddLine(a, b, thickness, color);
	}
}


void GLCanvas::OnPaint(wxPaintEvent &e)
{
	assert(m_context);

	m_context->SetCurrent(*this);

	wxPaintDC dc(this);
	
	const Color bg = Color(0.9f, 0.9f, 0.9f, 1.0f);

	m_context->Clear(bg);
	m_context->ClearBuffers();

	m_editor.OnDraw();

	m_context->CopyBuffers();
	m_context->SetMatrices(m_proj, m_view.GetMatrix());
	m_context->DrawElements();

	SwapBuffers();
}


void GLCanvas::OnSize(wxSizeEvent &e)
{
	e.Skip();

	assert(m_context);

	m_context->SetCurrent(*this);

	wxSize size = e.GetSize();

	glViewport(0, 0, size.x, size.y);

	float width = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);
	m_proj = glm::ortho(0.0f, width, height, 0.0f);
}
