
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
}

void ViewMatrix::Pan(wxPoint2DDouble delta)
{
	wxPoint2DDouble pan = m_pan + delta / m_zoom;
	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
		return;
	}
	m_pan = pan;
}

