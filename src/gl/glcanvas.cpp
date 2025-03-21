
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "src/notebook.hpp"
#include "src/gl/glcontext.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/toolbar.hpp"


GLCanvas::GLCanvas(Notebook *parent, const wxGLAttributes &attrs)
	: wxGLCanvas(parent, attrs),
	  m_editor(this),
	  m_view(this)
{
	wxASSERT(wxGLCanvas::IsDisplaySupported(attrs));

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

	ToolBar *tb = ToolBar::GetInstance();
	m_editor.OnToolSelect(tb->GetSelected());
}


GLCanvas::~GLCanvas()
{
	RemoveEventHandler(&m_view);
	RemoveEventHandler(&m_editor);
}


GLCanvas *GLCanvas::GetCurrent()
{
	GLCanvas *canvas = nullptr;
	Notebook *notebook = Notebook::GetInstance();

	size_t pagecount = notebook->GetPageCount();

	if(pagecount > 0) {
		int sel = notebook->GetSelection();
		if(sel < pagecount) {
			wxWindow *page = notebook->GetPage(sel);
			canvas = dynamic_cast<GLCanvas *>(page);
		}
	}

	return canvas;
}


void GLCanvas::OnMouse(wxMouseEvent &e)
{
	m_mousepos = m_view.MouseToWorld(e);
	Refresh(true);
}


void GLCanvas::OutlineRect(const Rect2D &rect, float thickness, const glm::vec4 &color)
{
	glm::vec2 lt = rect.mins;
	glm::vec2 rb = rect.maxs;
	glm::vec2 lb = { rect.mins.x, rect.maxs.y };
	glm::vec2 rt = { rect.maxs.x, rect.mins.y };

	thickness /= m_view.GetZoom();
	GLContext *ctx = GLContext::GetInstance();
	ctx->AddLine(lt, rt, thickness, color);
	ctx->AddLine(rt, rb, thickness, color);
	ctx->AddLine(rb, lb, thickness, color);
	ctx->AddLine(lb, lt, thickness, color);
}


void GLCanvas::DrawLine(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color)
{
	thickness /= m_view.GetZoom();
	GLContext::GetInstance()->AddLine(a, b, thickness, color);
}

void GLCanvas::TexturePoly(const glm::vec2 pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const glm::vec4 &color)
{
	GLContext::GetInstance()->AddPolygon(pts, npts, uv, texture, color);
}

void GLCanvas::TexturePoly(const ConvexPolygon &p, const glm::vec4 &color)
{
	GLContext::GetInstance()->AddPolygon(p, color);
}


void GLCanvas::DrawPoint(const glm::vec2 &pt, const glm::vec4 &color)
{
	float thickness = 1.0 / m_view.GetZoom();
	glm::vec2 mins = { pt.x - thickness * 3.0f, pt.y - thickness * 3.0f };
	glm::vec2 maxs = { pt.x + thickness * 3.0f, pt.y + thickness * 3.0f };

	GLContext *ctx = GLContext::GetInstance();
	ctx->AddRect(mins, maxs, BLACK); /* outline */
	mins += thickness; maxs -= thickness;
	ctx->AddRect(mins, maxs, color); /* foreground */
	mins += thickness; maxs -= thickness;
	ctx->AddRect(mins, maxs, BLACK); /* center */
}


void GLCanvas::OutlinePoly(const glm::vec2 points[], size_t npoints, float thickness, const glm::vec4 &color)
{
	thickness /= m_view.GetZoom();

	for(size_t i = 0; i < npoints; i++) {
		glm::vec2 a = points[i];
		glm::vec2 b = points[(i + 1) % npoints];
		GLContext::GetInstance()->AddLine(a, b, thickness, color);
	}
}


void GLCanvas::OnPaint(wxPaintEvent &e)
{
	GLContext *ctx = GLContext::GetInstance();

	ctx->SetCurrent(*this);

	wxPaintDC dc(this);
	
	const glm::vec4 bg = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

	ctx->Clear(bg);
	ctx->ClearBuffers();

	m_editor.OnDraw();

	ctx->CopyBuffers();
	ctx->SetMatrices(m_proj, m_view.GetMatrix());
	ctx->DrawElements();

	SwapBuffers();
}


void GLCanvas::OnSize(wxSizeEvent &e)
{
	e.Skip();

	GLContext::GetInstance()->SetCurrent(*this);

	wxSize size = e.GetSize();

	glViewport(0, 0, size.x, size.y);

	float width = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);
	m_proj = glm::ortho(0.0f, width, height, 0.0f);
}
