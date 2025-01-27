#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <wx/event.h>
#include "src/viewmatrix.hpp"


ViewMatrix::ViewMatrix()
{
	m_view = glm::identity<Matrix4>();
}


wxPoint ViewMatrix::WorldToScreen(Point2D world) const
{
	wxPoint screen;
	world = m_view * glm::vec4(world, 0.0, 1.0);
	screen.x = static_cast<int>(world.x);
	screen.y = static_cast<int>(world.y);
	return screen;
}


Point2D ViewMatrix::ScreenToWorld(wxPoint screen) const
{
	Point2D s;
	s.x = static_cast<float>(screen.x);
	s.y = static_cast<float>(screen.y);
	return inverse(m_view) * glm::vec4(s, 0.0, 1.0);
}


Point2D ViewMatrix::MouseToWorld(wxMouseEvent &e) const
{
	wxPoint mpos = e.GetPosition();
	return ScreenToWorld(e.GetPosition());
}


void ViewMatrix::SetupMatrix()
{
	glm::vec3 zoom = { m_zoom, m_zoom, 1.0 };
	glm::vec3 pan = { m_pan, 0.0 };

	m_view = glm::identity<Matrix4>();
	m_view = glm::scale(m_view, zoom);
	m_view = glm::translate(m_view, pan);
}


void ViewMatrix::Zoom(Point2D p, float factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	auto pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.x > MAX_PAN_X || pan.x < MIN_PAN_X ||
	   pan.y > MAX_PAN_Y || pan.y < MIN_PAN_Y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;

	SetupMatrix();
}


void ViewMatrix::Pan(Point2D delta)
{
	Point2D pan = m_pan + delta / m_zoom;
	if(pan.x > MAX_PAN_X || pan.x < MIN_PAN_X ||
	   pan.y > MAX_PAN_Y || pan.y < MIN_PAN_Y) {
		return;
	}
	m_pan = pan;
	SetupMatrix();
}


ViewMatrixCtrl::ViewMatrixCtrl(wxWindow *parent)
	: m_parent(parent), m_inpan(false)
{
	Bind(wxEVT_MIDDLE_DOWN, &ViewMatrixCtrl::OnMouseMiddleDown, this, wxID_ANY);
	Bind(wxEVT_MIDDLE_UP, &ViewMatrixCtrl::OnMouseMiddleUp, this, wxID_ANY);
	Bind(wxEVT_MOUSEWHEEL, &ViewMatrixCtrl::OnMouseWheel, this, wxID_ANY);
	Bind(wxEVT_MOTION, &ViewMatrixCtrl::OnMouseMotion, this, wxID_ANY);
}


void ViewMatrixCtrl::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	wxPoint wxpos = e.GetPosition();

	Point2D pos;
	pos.x = static_cast<float>(wxpos.x);
	pos.y = static_cast<float>(wxpos.y);

	if(m_inpan && !e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		m_inpan = false;
	}

	if(m_inpan) {
		Pan(pos - m_lastmousepos);
		m_lastmousepos = pos;
	}
}


void ViewMatrixCtrl::OnMouseWheel(wxMouseEvent &e)
{
	wxPoint wxpos = e.GetPosition();
	Point2D pos;
	pos.x = static_cast<float>(wxpos.x);
	pos.y = static_cast<float>(wxpos.y);

	if(e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {
		int rot = e.GetWheelRotation();
		if(rot == 0) {
			/* no scroll */
		}
		else if(rot > 0) { /* scroll up */
			Zoom(pos, 1.1f);
		}
		else { /* scroll down */
			Zoom(pos, 0.9f);
		}
	}

	e.Skip();
}


void ViewMatrixCtrl::OnMouseMiddleDown(wxMouseEvent &e)
{
	e.Skip(true);

	wxPoint wxpos = e.GetPosition();

	Point2D pos;
	pos.x = static_cast<float>(wxpos.x);
	pos.y = static_cast<float>(wxpos.y);

	m_lastmousepos = pos;
	m_inpan = true;
}


void ViewMatrixCtrl::OnMouseMiddleUp(wxMouseEvent &e)
{
	e.Skip(true);
	m_inpan = false;
}