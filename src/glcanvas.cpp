#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "src/glnotebook.hpp"
#include "src/glcanvas.hpp"

using namespace glm;

wxBEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
	EVT_PAINT(GLCanvas::OnPaint)
	EVT_MOUSE_EVENTS(GLCanvas::OnMouse)
	EVT_SIZE(GLCanvas::OnSize)
	EVT_KEY_DOWN(GLCanvas::OnKeyDown)
wxEND_EVENT_TABLE()


GLCanvas::GLCanvas(GLNoteBook *parent, const wxGLAttributes &attrs)
	: wxGLCanvas(parent, attrs),
	  m_parent(parent)
{
	wxSize size = GetSize() * GetContentScaleFactor();
	double width = static_cast<double>(size.x);
	double height = static_cast<double>(size.y);

	m_proj = ortho(0.0, width, height, 0.0);
	m_view = identity<mat4>();
}


void GLCanvas::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	m_parent->SetCurrent(this);

	wxSize size = GetSize() * GetContentScaleFactor();
	double width  = static_cast<double>(size.x);
	double height = static_cast<double>(size.y);

	m_proj = ortho(0.0, width, height, 0.0);
	m_view = SetupView(m_pan, m_zoom);

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(m_proj));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(m_view));

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_LINES);
	glColor4f(0.7f,0.7f,0.7f,1.0f);

	int spacing = 100;

	for(int i = (MIN_PAN.x/spacing); i < (MAX_PAN.x/spacing); i++) {
		glVertex2i(i * spacing, MIN_PAN.y);
		glVertex2i(i * spacing, MAX_PAN.y);

		glVertex2i(MIN_PAN.x, i * spacing);
		glVertex2i(MAX_PAN.x, i * spacing);
	}

	glEnd();

	/* red quad */
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		for(glm::vec2 sq : m_squares) {
			glVertex2f(sq.x, sq.y);
			glVertex2f(sq.x, sq.y + 100);
			glVertex2f(sq.x + 100, sq.y + 100);
			glVertex2f(sq.x + 100, sq.y);
		}
	glEnd();

	SwapBuffers();
}


mat4 GLCanvas::SetupView(vec2 pan, float zoom)
{
	mat4 view = identity<mat4>();
	view = scale(view, vec3(zoom, zoom, 1.0));
	view = translate(view, vec3(pan, 0.0));
	return view;
}



void GLCanvas::OnSize(wxSizeEvent &e)
{
	e.Skip();

	if(!IsShownOnScreen()) {
		return;
	}

	m_parent->SetCurrent(this);

	Refresh(false);
}


void GLCanvas::Zoom(vec2 p, float factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	vec2 pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x ||
		pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;
}

void GLCanvas::Pan(vec2 delta)
{
	m_pan += delta / m_zoom;
	m_pan.x = clamp(m_pan.x, MIN_PAN.x, MAX_PAN.x);
	m_pan.y = clamp(m_pan.y, MIN_PAN.y, MAX_PAN.y);
}

void GLCanvas::OnMouse(wxMouseEvent &e)
{
	static glm::vec2 last_fpos;

	wxPoint pos = e.GetPosition();

	vec2 fpos;
	fpos.x = static_cast<float>(pos.x);
	fpos.y = static_cast<float>(pos.y);

	m_view = SetupView(m_pan, m_zoom);
	vec2 spos = inverse(m_view) * vec4(fpos, 1.0, 1.0);

	wxString s;
	s.Printf("Mouse Postition: %f %f", fpos.x, fpos.y);

	wxFrame *mainframe = dynamic_cast<wxFrame *>(m_parent->GetParent());
	if(mainframe) {
		mainframe->SetStatusText(s);
	}

	if(e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {

		int rot = e.GetWheelRotation();

		if(rot == 0) { 
			/* no scroll */
		} else if(rot > 0) { /* scroll up */
			Zoom(fpos, 1.1f);
		} else { /* scroll down */
			Zoom(fpos, 0.9f);
		}
	}

	if(e.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		last_fpos = fpos;
	}
	
	if(e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		Pan(fpos - last_fpos);
		last_fpos = fpos;
	}

	if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
		m_squares.push_back(spos);
	}


	Refresh(false);
}


void GLCanvas::OnKeyDown(wxKeyEvent &e)
{
	return;
}
