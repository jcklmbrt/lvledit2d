
#include <cmath>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include <wx/pen.h>

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
	inv.Invert();
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
}

static bool LineLine(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double a = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
	double b = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));

	if(a > 0 && a < 1 && b > 0 && b < 1) {
		return true;
	} else {
		return false;
	}
}

static bool LineRect(double x0, double y0, double x1, double y1, wxRect2DDouble rect)
{
	wxPoint2DDouble lt = rect.GetLeftTop();
	wxPoint2DDouble rt = rect.GetRightTop();
	wxPoint2DDouble lb = rect.GetLeftBottom();
	wxPoint2DDouble rb = rect.GetRightBottom();

	return  LineLine(lt.m_x, lt.m_y, rt.m_x, rt.m_y, x0, y0, x1, y1) || /* top */
		LineLine(lb.m_x, lb.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1) || /* bottom */
		LineLine(lt.m_x, lt.m_y, lb.m_x, lb.m_y, x0, y0, x1, y1) || /* left */
		LineLine(rt.m_x, rt.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1);   /* right */
}

static wxRect RectAroundPoint(double x, double y, double size)
{
	wxRect r;
	double size_2 = size / 2.0;
	r.x = x - size_2;
	r.y = y - size_2;
	r.width  = size;
	r.height = size;
	return r;
}


void DrawPanel::DrawRect(wxPaintDC &dc, wxRect2DDouble r, wxColour color, bool tmp)
{
	wxPen pens[2] = { wxPen(*wxBLACK, 3), wxPen(color, 1) };
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	for(wxPen pen : pens) {
		wxPoint2DDouble lt, rb;
		lt.m_x = r.GetLeft();
		lt.m_y = r.GetTop();
		rb.m_x = r.GetRight();
		rb.m_y = r.GetBottom();

		wxPoint slt, srb;
		WorldToScreen(lt, slt);
		WorldToScreen(rb, srb);

		dc.SetPen(pen);

		if(tmp && pen.GetColour() == color) {
			dc.SetPen(wxPen(*wxRED, 1));
		}

		wxSize size;
		size.x = srb.x - slt.x;
		size.y = srb.y - slt.y;

		dc.DrawRectangle(slt, size);
	}
}


void DrawPanel::DrawGrid(wxPaintDC &dc)
{
	dc.SetPen(wxPen(wxColour(0, 0, 0), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	int spacing = m_gridspacing;

	wxSize size = GetSize();
	wxPoint s_mins = { 0, 0 };
	wxPoint s_maxs = { size.x, size.y };

	wxPoint2DDouble mins, maxs;
	ScreenToWorld(s_mins, mins);
	ScreenToWorld(s_maxs, maxs);

	mins.m_x -= m_gridspacing + std::remainder(mins.m_x, (double)m_gridspacing);
	mins.m_y -= m_gridspacing + std::remainder(mins.m_y, (double)m_gridspacing);

	for(double x = mins.m_x; x < maxs.m_x; x += m_gridspacing) {
		wxPoint wa, wb;
		WorldToScreen({ x, mins.m_y }, wa);
		WorldToScreen({ x, maxs.m_y }, wb);
		dc.DrawLine(wa, wb);
	}

	for(double y = mins.m_y; y < maxs.m_y; y += m_gridspacing) {
		wxPoint wa, wb;
		WorldToScreen({ mins.m_x, y }, wa);
		WorldToScreen({ maxs.m_x, y }, wb);
		dc.DrawLine(wa, wb);
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	SetupView();
	DrawGrid(dc);

	std::vector<wxPoint> s_points;
	
	for(const ConvexPolygon &p : m_polys) {
		
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		for(int i = 0; i < p.NumPoints(); i++) {
			wxPoint s_point;
			WorldToScreen(p.GetPoint(i), s_point);
			s_points.push_back(s_point);
		}

		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawPolygon(s_points.size(), s_points.data());

		if(p.ContainsPoint(m_mousepos)) {
			dc.SetPen(wxPen(*wxGREEN, 1));
		} else {
			dc.SetPen(wxPen(*wxWHITE, 1));
		}

		dc.DrawPolygon(s_points.size(), s_points.data());

		for(wxPoint s_point : s_points) {
			dc.SetPen(*wxTRANSPARENT_PEN);

			wxBrush brushes[] = {
				*wxBLACK_BRUSH,
				wxBrush(*wxWHITE),
				*wxBLACK_BRUSH
			};

			int sizes[] = {
				6, 4, 2
			};

			// highlight corners
			for(int i = 0; i < 3; i++) {
				dc.SetBrush(brushes[i]);
				int size = sizes[i];
				wxRect cr = RectAroundPoint(s_point.x, s_point.y, size);
				dc.DrawRectangle(cr);
			}
		}

		s_points.clear();
	}

	if(m_inedit == ToolBar::ID::QUAD) {
		DrawRect(dc, m_tmprect, *wxWHITE, true);
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

	m_mousepos = world_pos;

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar   *toolbar   = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	switch(tool->GetId())
	{
	case ToolBar::ID::SELECT:
		if(e.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
			if(m_inedit != ToolBar::ID::SELECT) {
				for(size_t i = 0; i < m_polys.size(); i++) {
					if(m_polys[i].ContainsPoint(world_pos)) {
						m_inedit = ToolBar::ID::SELECT;
						m_selectedpoly = i;
						m_editstart = world_pos;
					}
				}
			}
			else {
				wxPoint2DDouble delta = world_pos - m_editstart;
				m_editstart = world_pos;
				m_polys[m_selectedpoly].MoveBy(delta);
			}
		}
		else if(m_inedit == ToolBar::ID::SELECT) {
			FinishEdit();
		}
		break;
	case ToolBar::ID::QUAD:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
			if(m_inedit != ToolBar::ID::QUAD) {
				m_editstart = world_pos;

				if(m_snaptogrid) {
					double spacing = static_cast<double>(m_gridspacing);
					m_editstart.m_x -= fmodl(m_editstart.m_x, spacing);
					m_editstart.m_y -= fmodl(m_editstart.m_y, spacing);
				}

				m_tmprect = wxRect2DDouble();
				m_tmprect.SetCentre(m_editstart);
				m_inedit = ToolBar::ID::QUAD;
			}
			else {
				wxSize size = m_tmprect.GetSize();
				wxASSERT(size.x >= 0 && size.y >= 0);

				m_polys.push_back(m_tmprect);
				FinishEdit();
			}
		}
		else if(m_inedit == ToolBar::ID::QUAD) {
			double x = world_pos.m_x;
			double y = world_pos.m_y;

			if(m_snaptogrid) {
				double spacing = static_cast<double>(m_gridspacing);
				x -= fmodl(x, spacing);
				y -= fmodl(y, spacing);
			}

			if(x > m_editstart.m_x) {
				m_tmprect.SetRight(x);
			} else {
				m_tmprect.SetLeft(x);
			} 
			if(y > m_editstart.m_y) {
				m_tmprect.SetBottom(y);
			} else {
				m_tmprect.SetTop(y);
			}
		}
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
