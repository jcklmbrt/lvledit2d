#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "src/lvledit2d.hpp"
#include "src/glnotebook.hpp"
#include "src/glcanvas.hpp"
#include "src/toolbar.hpp"

using namespace glm;
using namespace std;


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
	float width  = static_cast<float>(size.x);
	float height = static_cast<float>(size.y);

	m_proj = ortho(0.0f, width, height, 0.0f);

	SetupView();

	glViewport(0, 0, size.x, size.y);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_POINT_SMOOTH);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(m_proj));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(m_view));

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLineWidth(1.0f);
	glBegin(GL_LINES);
		glColor4f(0.7f,0.7f,0.7f,1.0f);

		static int spacing = 100;

		for(int i = (MIN_PAN.x/spacing); i < (MAX_PAN.x/spacing); i++) {
			glVertex2i(i * spacing, MIN_PAN.y);
			glVertex2i(i * spacing, MAX_PAN.y);
			glVertex2i(MIN_PAN.x, i * spacing);
			glVertex2i(MAX_PAN.x, i * spacing);
		}
	glEnd();

	glLineWidth(3.0f);
	glColor4f(0.0f,0.0f,0.0f,1.0f);

	glBegin(GL_LINES);
		for(aabb_t b : m_squares) {
			/* left */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.mins.x, b.maxs.y);
			/* right */
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			/* top */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			/* bottom */
			glVertex2f(b.mins.x, b.maxs.y);
			glVertex2f(b.maxs.x, b.maxs.y);

			/* left */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.mins.x, b.maxs.y);
			/* right */
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			/* top */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			/* bottom */
			glVertex2f(b.mins.x, b.maxs.y);
			glVertex2f(b.maxs.x, b.maxs.y);
		}
	glEnd();
	
	glLineWidth(1.0f);
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	glBegin(GL_LINES);
		for(aabb_t b : m_squares) {
			/* left */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.mins.x, b.maxs.y);
			/* right */
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			/* top */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			/* bottom */
			glVertex2f(b.mins.x, b.maxs.y);
			glVertex2f(b.maxs.x, b.maxs.y);

			/* left */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.mins.x, b.maxs.y);
			/* right */
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			/* top */
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			/* bottom */
			glVertex2f(b.mins.x, b.maxs.y);
			glVertex2f(b.maxs.x, b.maxs.y);
		}
	glEnd();

	glPointSize(6.0f);
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin(GL_POINTS);
		for(aabb_t b : m_squares) {
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			glVertex2f(b.mins.x, b.maxs.y);
		}
	glEnd();


	glPointSize(4.0f);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glBegin(GL_POINTS);
		for(aabb_t b : m_squares) {
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			glVertex2f(b.mins.x, b.maxs.y);
		}
	glEnd();

	glPointSize(2.0f);
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin(GL_POINTS);
		for(aabb_t b : m_squares) {
			glVertex2f(b.mins.x, b.mins.y);
			glVertex2f(b.maxs.x, b.mins.y);
			glVertex2f(b.maxs.x, b.maxs.y);
			glVertex2f(b.mins.x, b.maxs.y);
		}
	glEnd();

	SwapBuffers();
}


void GLCanvas::SetupView()
{
	vec3 zoom = { m_zoom, m_zoom, 1.0 };
	vec3 pan  = { m_pan, 0.0 };

	m_view = identity<mat4>();
	m_view = scale(m_view, zoom);
	m_view = translate(m_view, pan);
}


vec2 GLCanvas::WorldToScreen(vec2 world)
{
	return m_view * vec4(world, 0.0, 1.0);
}


vec2 GLCanvas::ScreenToWorld(vec2 screen)
{
	return inverse(m_view) * vec4(screen, 0.0, 1.0);
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
	vec2 pan = m_pan + delta / m_zoom;
	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x ||
	   pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}
	m_pan = pan;
}

void GLCanvas::OnMouse(wxMouseEvent &e)
{
	static vec2 last_pos;

	wxPoint wxpos = e.GetPosition();

	vec2 pos;
	pos.x = static_cast<float>(wxpos.x);
	pos.y = static_cast<float>(wxpos.y);

	SetupView();
	vec2 world_pos = ScreenToWorld(pos);

	wxFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	static aabb_t *cur_sqr = nullptr;

	switch(tool->GetId())
	{
	case ToolBar::ID::QUAD:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
			if(cur_sqr == nullptr) {
				aabb_t bb;
				bb.mins = world_pos;
				bb.maxs = world_pos;
				m_squares.push_back(bb);
				cur_sqr = &m_squares.back();
			} else {
				cur_sqr->maxs = world_pos;
				cur_sqr = nullptr; 
			}
		} else if(cur_sqr != nullptr) {
			cur_sqr->maxs = world_pos;
		}
		break;
	default:
		break;
	}

	wxString s;
	s.Printf("%s (%f,%f)", tool->GetLabel(), world_pos.x, world_pos.y);

	mainframe->SetStatusText(s);

	if(e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {
		int rot = e.GetWheelRotation();
		if(rot == 0) { 
			/* no scroll */
		} else if(rot > 0) { /* scroll up */
			Zoom(pos, 1.1f);
		} else { /* scroll down */
			Zoom(pos, 0.9f);
		}
	}

	if(e.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		last_pos = pos;
	}
	
	if(e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		Pan(pos - last_pos);
		last_pos = pos;
	}

	Refresh(false);
}

void GLCanvas::OnKeyDown(wxKeyEvent &e)
{
	return;
}
