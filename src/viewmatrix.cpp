#include <wx/window.h>
#include "src/viewmatrix.hpp"

ViewMatrix::ViewMatrix()
{
	wxASSERT(m_matrix.IsIdentity());
}

wxPoint ViewMatrix::WorldToScreen(wxPoint2DDouble world)
{
	wxPoint screen;
	world = m_matrix.TransformPoint(world);
	screen.x = static_cast<int>(world.m_x);
	screen.y = static_cast<int>(world.m_y);
	return screen;
}

wxPoint2DDouble ViewMatrix::ScreenToWorld(wxPoint screen)
{
	wxPoint2DDouble world;
	world.m_x = static_cast<wxDouble>(screen.x);
	world.m_y = static_cast<wxDouble>(screen.y);

	wxAffineMatrix2D inv = m_matrix;
	inv.Invert();

	return inv.TransformPoint(world);
}

wxPoint2DDouble ViewMatrix::MouseToWorld(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	return ScreenToWorld(e.GetPosition());
}

void ViewMatrix::SetupMatrix()
{
	m_matrix = wxAffineMatrix2D();
	m_matrix.Scale(m_zoom, m_zoom);
	m_matrix.Translate(m_pan.m_x, m_pan.m_y);
}

void ViewMatrix::Zoom(wxPoint2DDouble p, wxDouble factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	auto pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;

	SetupMatrix();
}

void ViewMatrix::Pan(wxPoint2DDouble delta)
{
	wxPoint2DDouble pan = m_pan + delta / m_zoom;
	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
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

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

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
	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

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

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	m_lastmousepos = pos;
	m_inpan = true;
}

void ViewMatrixCtrl::OnMouseMiddleUp(wxMouseEvent &e)
{
	e.Skip(true);
	m_inpan = false;
}