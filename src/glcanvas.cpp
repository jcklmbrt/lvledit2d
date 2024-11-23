#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>

#include "src/glcanvas.hpp"


wxBEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
	EVT_PAINT(GLCanvas::OnPaint)
	EVT_MOUSE_EVENTS(GLCanvas::OnMouse)
	EVT_SIZE(GLCanvas::OnSize)
	EVT_KEY_DOWN(GLCanvas::OnKeyDown)
wxEND_EVENT_TABLE()


GLCanvas::GLCanvas(wxFrame *parent, const wxGLAttributes &attrs)
	: wxGLCanvas(parent, attrs),
	  m_parent(parent)
{
	wxGLContextAttrs ctx_attrs;
	ctx_attrs.PlatformDefaults().CoreProfile().OGLVersion(3, 0).EndList();
	m_context = new wxGLContext(this, 0, &ctx_attrs);

	if(!m_context->IsOK()) {
		wxMessageBox("OpenGL Version Error");
		delete m_context;
		m_context = nullptr;
	}


}


void GLCanvas::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	SetCurrent(*m_context);

	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* red quad */
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex2f(100, 100);
		glVertex2f(100, 200);
		glVertex2f(200, 200);
		glVertex2f(200, 100);
	glEnd();

	SwapBuffers();
}


void GLCanvas::OnSize(wxSizeEvent &e)
{
	e.Skip();

	if(!IsShownOnScreen()) {
		return;
	}

	SetCurrent(*m_context);

	wxSize size = e.GetSize() * GetContentScaleFactor();
	int width  = size.x;
	int height = size.y;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (GLdouble)width, (GLdouble)height, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Refresh(false);
}


void GLCanvas::OnMouse(wxMouseEvent &e)
{
	wxPoint pos = e.GetPosition();

	wxString s;
	s.Printf("Mouse Postition: %d %d", pos.x, pos.y);

	if(m_parent != nullptr) {
		m_parent->SetStatusText(s);
	}
}


void GLCanvas::OnKeyDown(wxKeyEvent &e)
{
	return;
}
