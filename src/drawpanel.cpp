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
	SetBackgroundColour(wxColour(0xFF, 0xFF, 0xEA));
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

		if(tmp) {
			bool intersects = false;
			for(const wxRect2DDouble &rect : m_rects) {
				if(rect.Intersects(r)) {
					intersects = true;
					break;
				}
			}
			if(intersects && pen.GetColour() == color) {
				dc.SetPen(wxPen(*wxRED, 1));
			}
		}

		wxSize size;
		size.x = srb.x - slt.x;
		size.y = srb.y - slt.y;

		dc.DrawRectangle(slt, size);
	}

	dc.SetPen(*wxTRANSPARENT_PEN);

	wxBrush brushes[] = {
		*wxBLACK_BRUSH,
		*wxRED_BRUSH,
		*wxBLACK_BRUSH
	};

	int sizes[] = {
		6, 4, 2
	};


	// highlight corners
	for(int i = 0; i < 3; i++) {
		dc.SetBrush(brushes[i]);
		wxPoint2DDouble lt, rb;
		lt.m_x = r.GetLeft();
		lt.m_y = r.GetTop();
		rb.m_x = r.GetRight();
		rb.m_y = r.GetBottom();

		wxPoint slt, srb;
		WorldToScreen(lt, slt);
		WorldToScreen(rb, srb);

		int size = sizes[i];

		wxRect corners[] = {
			RectAroundPoint(srb.x, slt.y, size),
			RectAroundPoint(slt.x, srb.y, size),
			RectAroundPoint(srb.x, srb.y, size),
			RectAroundPoint(slt.x, slt.y, size),
		};

		for(wxRect corner : corners) {
			dc.DrawRectangle(corner);
		}
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	SetupView();

	dc.SetPen(wxPen(wxColour(220, 220, 220), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	int spacing = m_gridspacing;
	for(int i = MIN_PAN_X / spacing; i < MAX_PAN_X / spacing; i++) {
		wxPoint wa, wb;
		double  space = static_cast<double>(i * spacing);
		WorldToScreen({ space, MIN_PAN_Y }, wa);
		WorldToScreen({ space, MAX_PAN_Y }, wb);
		dc.DrawLine(wa, wb);
		WorldToScreen({ MIN_PAN_X, space }, wa);
		WorldToScreen({ MAX_PAN_Y, space }, wb);
		dc.DrawLine(wa, wb);
	}

	for(const wxRect2DDouble &r : m_rects) {
		DrawRect(dc, r, *wxGREEN, false);
	}

	for(const ConnectLine line : m_lines) {
		wxPoint2DDouble p[2];
		wxPoint         s[2];
		for(int i = 0; i < 2; i++) {
			wxRect2DDouble &r = m_rects[line.rect[i]];
			switch(line.outcode[i]) {
			case wxOutLeft  + wxOutTop:    p[i] = r.GetLeftTop();     break;
			case wxOutRight + wxOutTop:    p[i] = r.GetRightTop();    break;
			case wxOutLeft  + wxOutBottom: p[i] = r.GetLeftBottom();  break;
			case wxOutRight + wxOutBottom: p[i] = r.GetRightBottom(); break;
			}
			WorldToScreen(p[i], s[i]);
		}

		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawLine(s[0], s[1]);
		dc.SetPen(wxPen(*wxGREEN, 1));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawLine(s[0], s[1]);

	}

	if(m_inedit == ToolBar::ID::QUAD) {
		DrawRect(dc, m_tmprect, *wxWHITE, true);
	}

	if(m_inedit == ToolBar::ID::LINE) {
		int corner, rect;
		bool good = false;
		wxPoint2DDouble closest_point = m_mousepos;
		if(FindClosestRectCorner(closest_point, corner, rect)) {
			if(rect != m_tmpline.rect[0]) {
				wxPoint a, b;
				WorldToScreen(closest_point, a);
				WorldToScreen(m_editstart, b);
				dc.SetPen(wxPen(*wxBLACK, 3));
				dc.SetBrush(*wxTRANSPARENT_BRUSH);
				dc.DrawLine(a, b);
				dc.SetPen(wxPen(*wxWHITE, 1));
				dc.SetBrush(*wxTRANSPARENT_BRUSH);
				dc.DrawLine(a, b);
				good = true;
			}
		}
		if(good == false) {
			wxPoint a, b;
			WorldToScreen(m_mousepos, a);
			WorldToScreen(m_editstart, b);
			dc.SetPen(wxPen(*wxBLACK, 3));
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawLine(a, b);
			dc.SetPen(wxPen(*wxRED, 1));
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawLine(a, b);
		}
	}
}

bool DrawPanel::FindClosestRectCorner(wxPoint2DDouble &pt, int &corner, int &rect)
{
	int    closest_rect   = 0;
	int    closest_corner = wxInside;
	double closest_dist   = m_gridspacing * m_gridspacing;
	wxPoint2DDouble closest_point;

	int num_rects = m_rects.size();
	for(int i = 0; i < num_rects; i++) {

		int corners[] = {
			wxOutLeft  + wxOutTop,
			wxOutRight + wxOutTop,
			wxOutLeft  + wxOutBottom,
			wxOutRight + wxOutBottom
		};

		wxPoint2DDouble points[] = {
			m_rects[i].GetLeftTop(),
			m_rects[i].GetRightTop(),
			m_rects[i].GetLeftBottom(),
			m_rects[i].GetRightBottom()
		};

		/* for each corner */
		for(int j = 0; j < 4; j++) {
			double dist = pt.GetDistanceSquare(points[j]);
			if(dist < closest_dist) {
				closest_point  = points[j];
				closest_corner = corners[j];
				closest_rect = i;
				closest_dist = dist;
			}
		}
	}

	if(closest_corner != wxInside) {
		corner = closest_corner;
		rect   = closest_rect;
		pt     = closest_point;
		return true;
	} else {
		return false;
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

	wxFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	switch(tool->GetId())
	{
	case ToolBar::ID::LINE:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
			if(m_inedit != ToolBar::ID::LINE) {
				int corner, rect;
				wxPoint2DDouble closest_point = world_pos;
				if(FindClosestRectCorner(closest_point, corner, rect)) {
					m_inedit = ToolBar::ID::LINE;
					m_editstart = closest_point;
					m_tmpline.outcode[0] = corner;
					m_tmpline.rect[0]    = rect;
				}
			} else {
				int corner, rect;
				wxPoint2DDouble closest_point = world_pos;
				if(FindClosestRectCorner(closest_point, corner, rect)) {
					if(rect != m_tmpline.rect[0]) {
						m_editstart = closest_point;
						m_tmpline.outcode[1] = corner;
						m_tmpline.rect[1]    = rect;
						m_lines.push_back(m_tmpline);
						FinishEdit();
					}
				}
			}
		}
		break;
	case ToolBar::ID::QUAD:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
			if(m_inedit != ToolBar::ID::QUAD) {
				bool intersects = false;
				for(const wxRect2DDouble &rect : m_rects) {
					if(rect.Contains(world_pos)) {
						intersects = true;
						break;
					}
				}
				if(!intersects) {

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
			}
			else {
				wxSize size = m_tmprect.GetSize();
				wxASSERT(size.x >= 0 && size.y >= 0);
				bool intersects = false;
				for(const wxRect2DDouble &rect : m_rects) {
					if(rect.Intersects(m_tmprect)) {
						intersects = true;
						break;
					}
				}
				if(!intersects) {
					m_rects.push_back(m_tmprect);
					FinishEdit();
				}
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
	default:
		if(m_inedit != 0) {
			FinishEdit();
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
