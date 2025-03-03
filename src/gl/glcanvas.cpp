
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
	  editor(this),
	  view(this)
{
	wxASSERT(wxGLCanvas::IsDisplaySupported(attrs));

	wxSize size = GetSize() * GetContentScaleFactor();

	float width = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);
	proj = glm::ortho(0.0f, width, height, 0.0f);

	Bind(wxEVT_SIZE, &GLCanvas::OnSize, this);
	Bind(wxEVT_PAINT, &GLCanvas::OnPaint, this);

	Bind(wxEVT_LEFT_DOWN, &GLCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &GLCanvas::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &GLCanvas::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &GLCanvas::OnMouse, this);
	Bind(wxEVT_MOTION, &GLCanvas::OnMouse, this);

	PushEventHandler(&editor);
	PushEventHandler(&view);

	ToolBar *tb = ToolBar::GetInstance();
	editor.OnToolSelect(tb->selected);
}


GLCanvas::~GLCanvas()
{
	RemoveEventHandler(&view);
	RemoveEventHandler(&editor);
}


GLCanvas *GLCanvas::GetCurrent()
{
	GLCanvas *canvas = nullptr;
	Notebook *notebook = Notebook::GetInstance();

	if(notebook->GetPageCount() > 0) {
		int sel = notebook->GetSelection();
		wxWindow *page = notebook->GetPage(sel);
		canvas = dynamic_cast<GLCanvas *>(page);
	}

	return canvas;
}


void GLCanvas::OnMouse(wxMouseEvent &e)
{
	mousepos = view.MouseToWorld(e);
	Refresh(true);
}


void GLCanvas::OutlineRect(const Rect2D &rect, float thickness, const glm::vec4 &color)
{
	glm_vec2 lt = rect.GetLeftTop();
	glm_vec2 lb = rect.GetLeftBottom();
	glm_vec2 rb = rect.GetRightBottom();
	glm_vec2 rt = rect.GetRightTop();

	thickness /= view.GetZoom();
	GLContext *ctx = GLContext::GetInstance();
	ctx->AddLine(lt, rt, thickness, color);
	ctx->AddLine(rt, rb, thickness, color);
	ctx->AddLine(rb, lb, thickness, color);
	ctx->AddLine(lb, lt, thickness, color);
}


void GLCanvas::DrawLine(const glm_vec2 &a, const glm_vec2 &b, float thickness, const glm::vec4 &color)
{
	thickness /= view.GetZoom();
	GLContext::GetInstance()->AddLine(a, b, thickness, color);
}

void GLCanvas::TexturePoly(const glm_vec2 pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const glm::vec4 &color)
{
	GLContext::GetInstance()->AddPolygon(pts, npts, uv, texture, color);
}

void GLCanvas::TexturePoly(const ConvexPolygon &p, const glm::vec4 &color)
{
	GLContext::GetInstance()->AddPolygon(p, color);
}


void GLCanvas::DrawPoint(const glm_vec2 &pt, const glm::vec4 &color)
{
	float thickness = 1.0 / view.GetZoom();
	glm_vec2 mins = { pt.x - thickness * 3.0f, pt.y - thickness * 3.0f };
	glm_vec2 maxs = { pt.x + thickness * 3.0f, pt.y + thickness * 3.0f };
	Rect2D rect = Rect2D(mins, maxs);

	GLContext *ctx = GLContext::GetInstance();
	ctx->AddRect(rect, BLACK); /* outline */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	ctx->AddRect(rect, color); /* foreground */
	rect.Inset(thickness * 2.0, thickness * 2.0);
	ctx->AddRect(rect, BLACK); /* center */
}


void GLCanvas::OutlinePoly(const glm_vec2 points[], size_t npoints, float thickness, const glm::vec4 &color)
{
	thickness /= view.GetZoom();

	for(size_t i = 0; i < npoints; i++) {
		glm_vec2 a = points[i];
		glm_vec2 b = points[(i + 1) % npoints];
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

	editor.OnDraw();

	ctx->CopyBuffers();
	ctx->SetMatrices(proj, view.GetMatrix());
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
	proj = glm::ortho(0.0f, width, height, 0.0f);
}
