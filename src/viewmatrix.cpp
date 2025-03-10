#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <wx/event.h>
#include "src/viewmatrix.hpp"


ViewMatrixBase::ViewMatrixBase()
{
	SetupMatrix();
}


wxPoint ViewMatrixBase::WorldToScreen(glm::vec2 world) const
{
	wxPoint screen;
	world = m_view * glm::vec4(world, 0.0, 1.0);
	screen.x = static_cast<int>(world.x);
	screen.y = static_cast<int>(world.y);
	return screen;
}


glm::vec2 ViewMatrixBase::ScreenToWorld(wxPoint screen) const
{
	glm::vec2 s;
	s.x = static_cast<float>(screen.x);
	s.y = static_cast<float>(screen.y);
	return inverse(m_view) * glm::vec4(s, 0.0, 1.0);
}


glm::vec2 ViewMatrixBase::MouseToWorld(wxMouseEvent &e) const
{
	wxPoint mpos = e.GetPosition();
	return ScreenToWorld(e.GetPosition());
}


void ViewMatrixBase::SetupMatrix()
{
	glm::vec3 zoom = { m_zoom, m_zoom, 1.0 };
	glm::vec3 pan = { m_pan, 0.0 };

	m_view = glm::identity<glm::mat4>();
	m_view = glm::scale(m_view, zoom);
	m_view = glm::translate(m_view, pan);
}


void ViewMatrixBase::Zoom(glm::vec2 p, float factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	auto pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x ||
	   pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;

	SetupMatrix();
}


void ViewMatrixBase::Pan(glm::vec2 delta)
{
	glm::vec2 pan = m_pan + delta / m_zoom;
	if(pan.x > MAX_PAN.x || pan.x < MIN_PAN.x ||
	   pan.y > MAX_PAN.y || pan.y < MIN_PAN.y) {
		return;
	}
	m_pan = pan;
	SetupMatrix();
}


ViewMatrixCtrl::ViewMatrixCtrl(wxWindow *parent)
	: m_parent(parent), m_inpan(false)
{
	Bind(wxEVT_MIDDLE_DOWN, &ViewMatrixCtrl::OnMouseMiddleDown, this);
	Bind(wxEVT_MIDDLE_UP, &ViewMatrixCtrl::OnMouseMiddleUp, this);
	Bind(wxEVT_MOUSEWHEEL, &ViewMatrixCtrl::OnMouseWheel, this);
	Bind(wxEVT_MOTION, &ViewMatrixCtrl::OnMouseMotion, this);
}


void ViewMatrixCtrl::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	wxPoint wxpos = e.GetPosition();

	glm::vec2 pos;
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
	glm::vec2 pos;
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

	glm::vec2 pos;
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
