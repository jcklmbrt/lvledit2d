#include "src/lvledit2d.hpp"
#include "src/toolbar.hpp"
#include "src/drawpanel.hpp"

wxBEGIN_EVENT_TABLE(DrawPanel, wxPanel)
	EVT_PAINT(DrawPanel::OnPaint)
	EVT_MOUSE_EVENTS(DrawPanel::OnMouse)
	EVT_KEY_DOWN(DrawPanel::OnKeyDown)
wxEND_EVENT_TABLE()

void DrawPanel::WorldToScreen(wxPoint2DDouble world, wxPoint &screen)
{
	world = m_view.TransformPoint(world);
	screen.x = static_cast<int>(world.m_x);
	screen.y = static_cast<int>(world.m_y);
}

void DrawPanel::ScreenToWorld(wxPoint screen, wxPoint2DDouble &world)
{
	world.m_x = static_cast<wxDouble>(screen.x);
	world.m_y = static_cast<wxDouble>(screen.y);

	wxAffineMatrix2D inv = m_view;
	bool inverted = inv.Invert();
	wxASSERT(inverted);
	/* ohh, it's a const reference! */
	world = inv.TransformPoint(world);
}

void DrawPanel::SetupView()
{
	m_view = wxAffineMatrix2D();
	m_view.Scale(m_zoom, m_zoom);
	m_view.Translate(m_pan.m_x, m_pan.m_y);
}

void DrawPanel::Zoom(wxPoint2DDouble p, wxDouble factor)
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

void DrawPanel::Pan(wxPoint2DDouble delta)
{
	wxPoint2DDouble pan = m_pan + delta / m_zoom;
	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
		return;
	}
	m_pan = pan;
}

DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent)
{
	wxASSERT(m_view.IsIdentity() == true);
	SetBackgroundColour(wxColour(77, 77, 77));
}

void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	SetupView();
	dc.SetTransformMatrix(m_view);

	static int spacing = 100;
	for(int i = MIN_PAN_X / spacing; i < MAX_PAN_X / spacing; i++) {
		dc.DrawLine(i * spacing, MIN_PAN_Y,
		            i * spacing, MAX_PAN_Y);
		dc.DrawLine(MIN_PAN_X, i * spacing,
		            MAX_PAN_X, i * spacing);
	}

	for(wxRect r : m_rects) {
		dc.DrawRectangle(r);
	}
}

void DrawPanel::OnMouse(wxMouseEvent &e)
{
	static wxPoint2DDouble last_pos;

	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	SetupView();
	wxPoint2DDouble world_pos;
	ScreenToWorld(wxpos, world_pos);

	wxFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	static wxRect *cur_rect = nullptr;

	switch(tool->GetId())
	{
	case ToolBar::ID::QUAD:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {

			if(cur_rect == nullptr) {
				wxRect r;
				r.SetX(world_pos.m_x);
				r.SetY(world_pos.m_y);

				m_rects.push_back(r);
				cur_rect = &m_rects.back();
			}
			else {
				cur_rect->SetRight(world_pos.m_x);
				cur_rect->SetBottom(world_pos.m_y);
				cur_rect = nullptr;
			}
		}
		else if(cur_rect != nullptr) {
			cur_rect->SetRight(world_pos.m_x);
			cur_rect->SetBottom(world_pos.m_y);
		}
		break;
	default:
		break;
	}

	wxString s;
	s.Printf("%s (%f,%f)", tool->GetLabel(), world_pos.m_x, world_pos.m_y);

	mainframe->SetStatusText(s);

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

	if(e.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		last_pos = pos;
	}

	if(e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		Pan(pos - last_pos);
		last_pos = pos;
	}

	Refresh(false);
}

void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}