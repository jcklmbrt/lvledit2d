#include <algorithm>

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

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glScalef(m_zoom, m_zoom, 1.0f);
	glTranslatef(m_pan.x, m_pan.y, 0.0);

	/* red quad */
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex2f(100, 100);
		glVertex2f(100, 200);
		glVertex2f(200, 200);
		glVertex2f(200, 100);
	glEnd();

	glPopMatrix();

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


void GLCanvas::Zoom(float factor)
{
	m_zoom *= factor;
	m_zoom = std::clamp(m_zoom, MIN_ZOOM, MAX_ZOOM);
}

void GLCanvas::ZoomAt(glm::vec2 p, float factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	glm::vec2 pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x ||
	   pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}

	m_zoom = zoom;
	m_pan  = pan;
}

void GLCanvas::Pan(glm::vec2 delta)
{
	m_pan += delta / m_zoom;
	m_pan.x = std::clamp(m_pan.x, MIN_PAN.x, MAX_PAN.x);
	m_pan.y = std::clamp(m_pan.y, MIN_PAN.y, MAX_PAN.y);
}

void GLCanvas::OnMouse(wxMouseEvent &e)
{
	static glm::vec2 last_fpos;

	wxPoint pos = e.GetPosition();

	wxString s;
	s.Printf("Mouse Postition: %d %d", pos.x, pos.y);

	if(m_parent != nullptr) {
		m_parent->SetStatusText(s);
	}

	glm::vec2 fpos;
	fpos.x = static_cast<float>(pos.x);
	fpos.y = static_cast<float>(pos.y);

	if(e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {

		int rot = e.GetWheelRotation();

		if(rot == 0) { 
			/* no scroll */
		} else if(rot > 0) { /* scroll up */
			ZoomAt(fpos, 1.1f);
		} else { /* scroll down */
			ZoomAt(fpos, 0.9f);
		}
	}

	if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
		last_fpos = fpos;
	}
	
	if(e.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
		Pan(fpos - last_fpos);
		last_fpos = fpos;
	}


	Refresh(false);
}


void GLCanvas::OnKeyDown(wxKeyEvent &e)
{
	return;
}
