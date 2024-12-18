#include <cmath>
#include <wx/debug.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include <wx/pen.h>

#include "box2d/collision.h"
#include "box2d/math_functions.h"
#include "src/lvledit2d.hpp"
#include "src/toolbar.hpp"
#include "src/drawpanel.hpp"
#include "src/viewmatrix.hpp"

#include "src/edit/ibaseedit.hpp"
#include "src/edit/rectangle.hpp"
#include "src/edit/selection.hpp"
#include "src/edit/line.hpp"

DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent)
{
	Bind(wxEVT_PAINT,    &DrawPanel::OnPaint, this, wxID_ANY);
	Bind(wxEVT_KEY_DOWN, &DrawPanel::OnKeyDown, this, wxID_ANY);

	Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnMouseLeftDown, this, wxID_ANY);
	Bind(wxEVT_LEFT_UP, &DrawPanel::OnMouseLeftUp, this, wxID_ANY);

	Bind(wxEVT_MOTION, &DrawPanel::OnMouseMotion, this, wxID_ANY);

	/* Zoom/Pan ctrl */
	Bind(wxEVT_MIDDLE_DOWN, &DrawPanel::OnMouseMiddleDown, this, wxID_ANY);
	Bind(wxEVT_MIDDLE_UP,   &DrawPanel::OnMouseMiddleUp, this, wxID_ANY);
	Bind(wxEVT_MOUSEWHEEL,  &DrawPanel::OnMouseWheel, this, wxID_ANY);
}

void DrawPanel::FinishEdit()
{
	if(m_edit != nullptr) {
		delete m_edit;
	}

	m_edit = nullptr;
}

static bool wxPointInPolygon(wxPoint2DDouble wxpoint, b2Polygon &poly)
{
	b2Vec2 b2pos;
	b2pos.x = static_cast<float>(wxpoint.m_x);
	b2pos.y = static_cast<float>(wxpoint.m_y);
	return b2PointInPolygon(b2pos, &poly);
}

bool DrawPanel::SelectPoly(wxPoint2DDouble wpos, size_t &idx)
{
	size_t num_polys = size();
	for(size_t i = 0; i < num_polys; i++) {
		b2Polygon &p = at(i);
		if(wxPointInPolygon(wpos, p)) {
			idx = i;
			return true;
		}
	}

	return false;
}

void DrawPanel::DrawGrid(wxPaintDC &dc)
{
	/* TODO: find a better color */
	dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxSize  size   = GetSize();
	wxPoint s_mins = { 0, 0 };
	wxPoint s_maxs = { size.x, size.y };

	wxPoint2DDouble mins, maxs;
	mins = ScreenToWorld(s_mins);
	maxs = ScreenToWorld(s_maxs);

	int spacing = m_gridspacing;

	mins.m_x -= spacing + std::remainder(mins.m_x, (double)spacing);
	mins.m_y -= spacing + std::remainder(mins.m_y, (double)spacing);

	for(double x = mins.m_x; x < maxs.m_x; x += spacing) {
		wxPoint wa = WorldToScreen({ x, mins.m_y });
		wxPoint wb = WorldToScreen({ x, maxs.m_y });
		dc.DrawLine(wa, wb);
	}

	for(double y = mins.m_y; y < maxs.m_y; y += spacing) {
		wxPoint wa = WorldToScreen({ mins.m_x, y });
		wxPoint wb = WorldToScreen({ maxs.m_x, y });
		dc.DrawLine(wa, wb);
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	SetupMatrix();
	DrawGrid(dc);

	std::vector<wxPoint> s_points;
	
	for(b2Polygon &p : *this) {
		
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		for(int i = 0; i < p.count; i++) {
			wxPoint2DDouble point = { p.vertices[i].x, p.vertices[i].y };
			wxPoint s_point = WorldToScreen(point);
			s_points.push_back(s_point);
		}

		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawPolygon(s_points.size(), s_points.data());

		if(wxPointInPolygon(m_mousepos, p)) {
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
				int size   = sizes[i];
				int size_2 = size / 2;

				wxRect cr;
				cr.SetWidth(size);
				cr.SetHeight(size);
				cr.SetX(s_point.x - size_2);
				cr.SetY(s_point.y - size_2);
				dc.DrawRectangle(cr);
			}
		}

		s_points.clear();
	}

	if(m_edit != nullptr) {
		m_edit->OnPaint(dc);
	}
}

void DrawPanel::OnMouseLeftDown(wxMouseEvent &e)
{
	if(m_edit == nullptr) {
		MainFrame *mainframe = wxGetApp().GetMainFrame();
		ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
		wxToolBarToolBase *tool = toolbar->GetSelected();

		switch(tool->GetId()) {
		case ToolBar::ID::SELECT:
			m_edit = new SelectionEdit(this);
			break;
		case ToolBar::ID::QUAD:
			m_edit = new RectangleEdit(this);
			break;
		case ToolBar::ID::LINE:
			m_edit = new LineEdit(this);
			break;
		default:
			FinishEdit();
			break;
		}
	}
		
	if(m_edit != nullptr) {
		m_edit->OnMouseLeftDown(e);
	}

	Refresh(false);
}

void DrawPanel::OnMouseLeftUp(wxMouseEvent &e)
{
	if(m_edit != nullptr) {
		m_edit->OnMouseLeftUp(e);
	}
}

void DrawPanel::OnMouseMotion(wxMouseEvent &e)
{
	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	wxPoint2DDouble wpos = ScreenToWorld(wxpos);
	m_mousepos = wpos;

	if(m_edit != nullptr) {
		m_edit->OnMouseMotion(e);
	}

	if(m_inpan && !e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		m_inpan = false;
	}

	if(m_inpan) {
		Pan(pos - m_lastmousepos);
		m_lastmousepos = pos;
	}

	Refresh(false);
}

void DrawPanel::OnMouseWheel(wxMouseEvent &e)
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

	Refresh(false);
}

void DrawPanel::OnMouseMiddleDown(wxMouseEvent &e) 
{
	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	m_lastmousepos = pos;
	m_inpan = true;
}

void DrawPanel::OnMouseMiddleUp(wxMouseEvent &e)
{
	m_inpan = false;
}

void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}
