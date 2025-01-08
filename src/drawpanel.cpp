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

#include "src/edit/ibaseedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/lineedit.hpp"

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

DrawPanel::~DrawPanel()
{
	FinishEdit();
}

void DrawPanel::FinishEdit()
{
	if(m_edit != nullptr) {
		RemoveEventHandler(m_edit);
		delete m_edit;
	}

	m_edit = nullptr;
}

ConvexPolygon *DrawPanel::SelectPoly(wxPoint2DDouble wpos)
{
	size_t num_polys = size();
	for(size_t i = 0; i < num_polys; i++) {
		ConvexPolygon &p = at(i);
		if(p.ContainsPoint(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


ConvexPolygon *DrawPanel::ClosestPoly(wxPoint2DDouble wpos, double threshold)
{
	ConvexPolygon *poly = nullptr;
	double min_dist = threshold;
	size_t num_polys = size();
	for(size_t i = 0; i < num_polys; i++) {
		ConvexPolygon &p = at(i);
		wxPoint2DDouble d = p.GetCenter() - wpos;
		double dist = d.GetVectorLength();
		if(dist < min_dist) {
			min_dist = dist;
			poly = &p;
		}
	}

	return poly;
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

void DrawPanel::DrawPoint(wxPaintDC &dc, wxPoint point, const wxColor *color)
{
	wxBrush brushes[] = {
		*wxBLACK_BRUSH,
		wxBrush(*color),
		*wxBLACK_BRUSH
	};

	int sizes[] = {
		6, 4, 2
	};

	dc.SetPen(*wxTRANSPARENT_PEN);

	// highlight corners
	for(int i = 0; i < 3; i++) {
		dc.SetBrush(brushes[i]);
		int size = sizes[i];
		int size_2 = size / 2;

		wxRect cr;
		cr.SetWidth(size);
		cr.SetHeight(size);
		cr.SetX(point.x - size_2);
		cr.SetY(point.y - size_2);
		dc.DrawRectangle(cr);
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	SetupMatrix();
	DrawGrid(dc);

	std::vector<wxPoint> s_points;

	for(ConvexPolygon &p : *this) {

		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		for(int i = 0; i < p.NumPoints(); i++) {
			wxPoint2DDouble point = p.GetPoint(i);
			wxPoint s_point = WorldToScreen(point);
			s_points.push_back(s_point);
		}

		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawPolygon(s_points.size(), s_points.data());

		if(p.ContainsPoint(m_mousepos)) {
			dc.SetPen(wxPen(*wxGREEN, 1));
		}
		else {
			dc.SetPen(wxPen(*wxWHITE, 1));
		}

		dc.DrawPolygon(s_points.size(), s_points.data());

		for(wxPoint s_point : s_points) {
			DrawPoint(dc, s_point, wxWHITE);
		}

		wxPoint2DDouble center = p.GetCenter();
		wxPoint s_center = WorldToScreen(center);

		DrawPoint(dc, s_center, wxYELLOW);

		s_points.clear();
	}
}

void DrawPanel::OnToolSelect(ToolBar::ID id)
{
	if(m_edit != nullptr) {
		FinishEdit();
		Refresh(false);
	}

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
			break;
		}

		if(m_edit != nullptr) {
			PushEventHandler(m_edit);
		}
	}
}


void DrawPanel::OnMouseLeftDown(wxMouseEvent &e)
{		
	Refresh(false);
}

void DrawPanel::OnMouseLeftUp(wxMouseEvent &e)
{
}

void DrawPanel::OnMouseMotion(wxMouseEvent &e)
{
	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	wxPoint2DDouble wpos = ScreenToWorld(wxpos);
	m_mousepos = wpos;

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
