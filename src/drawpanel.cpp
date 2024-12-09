
#include <cmath>
#include <wx/debug.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include <wx/pen.h>

#include "src/lvledit2d.hpp"
#include "src/toolbar.hpp"
#include "src/drawpanel.hpp"
#include "src/viewmatrix.hpp"


wxBEGIN_EVENT_TABLE(DrawPanel, wxPanel)
	EVT_PAINT(DrawPanel::OnPaint)
	EVT_MOUSE_EVENTS(DrawPanel::OnMouse)
	EVT_KEY_DOWN(DrawPanel::OnKeyDown)
wxEND_EVENT_TABLE()


DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent)
{
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
		slt = m_view.WorldToScreen(lt);
		srb = m_view.WorldToScreen(rb);

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
	/* TODO: find a better color */
	dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	int spacing = m_gridspacing;

	wxSize size = GetSize();
	wxPoint s_mins = { 0, 0 };
	wxPoint s_maxs = { size.x, size.y };

	wxPoint2DDouble mins, maxs;
	mins = m_view.ScreenToWorld(s_mins);
	maxs = m_view.ScreenToWorld(s_maxs);

	mins.m_x -= m_gridspacing + std::remainder(mins.m_x, (double)m_gridspacing);
	mins.m_y -= m_gridspacing + std::remainder(mins.m_y, (double)m_gridspacing);

	for(double x = mins.m_x; x < maxs.m_x; x += m_gridspacing) {
		wxPoint wa = m_view.WorldToScreen({ x, mins.m_y });
		wxPoint wb = m_view.WorldToScreen({ x, maxs.m_y });
		dc.DrawLine(wa, wb);
	}

	for(double y = mins.m_y; y < maxs.m_y; y += m_gridspacing) {
		wxPoint wa = m_view.WorldToScreen({ mins.m_x, y });
		wxPoint wb = m_view.WorldToScreen({ maxs.m_x, y });
		dc.DrawLine(wa, wb);
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	m_view.SetupMatrix();

	DrawGrid(dc);

	std::vector<wxPoint> s_points;
	
	for(const ConvexPolygon &p : m_polys) {
		
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		for(int i = 0; i < p.NumPoints(); i++) {
			wxPoint s_point = m_view.WorldToScreen(p.GetPoint(i));
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

	m_view.SetupMatrix();
	wxPoint2DDouble world_pos;
	world_pos = m_view.ScreenToWorld(wxpos);

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
			m_view.Zoom(pos, 1.1f);
		}
		else { /* scroll down */
			m_view.Zoom(pos, 0.9f);
		}
	}

	if(e.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		last_pos = pos;
	}

	if(e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		m_view.Pan(pos - last_pos);
		last_pos = pos;
	}

	Refresh(false);
}

void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}
