#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>

#include "src/drawpanel.hpp"
#include "src/edit/lineedit.hpp"


void LineEdit::StartPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble start = m_panel->ScreenToWorld(mpos);
	ConvexPolygon  *poly = m_context->GetSelectedPoly();

	if(poly == nullptr) {
		
	} else {
		if(m_panel->IsSnapToGrid()) {
			BackgroundGrid::Snap(start);
		}
		m_start = start;
		m_context->SetSelectedPoly(poly);
		m_state = LineEditState_t::END_POINT;
	}
}


void LineEdit::StartPoint_OnPaint(wxPaintDC &dc)
{
	wxPoint2DDouble mpos = m_panel->GetMousePos();
	if(m_panel->IsSnapToGrid()) {
		BackgroundGrid::Snap(mpos);
	}
	wxPoint spos = m_panel->WorldToScreen(mpos);
	DrawPanel::DrawPoint(dc, spos, wxWHITE);
}


void LineEdit::EndPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	m_end = m_panel->MouseToWorld(e);

	if(m_panel->IsSnapToGrid()) {
		BackgroundGrid::Snap(m_end);
	}

	m_plane = Plane2D(m_start, m_end);

	ConvexPolygon *poly = m_context->GetSelectedPoly();

	if(poly != nullptr) {
		m_points.clear();
		poly->ImposePlane(m_plane, m_points);
		m_state = LineEditState_t::SLICE;
	}
}


void LineEdit::EndPoint_OnPaint(wxPaintDC &dc)
{
	wxPoint a, b;
	wxPoint2DDouble mpos = m_panel->GetMousePos();

	a = m_panel->WorldToScreen(m_start);

	if(m_panel->IsSnapToGrid()) {
		BackgroundGrid::Snap(mpos);
	}

	b = m_panel->WorldToScreen(mpos);

	dc.SetPen(wxPen(*wxBLACK, 2));
	dc.DrawLine(a, b);
	dc.SetPen(wxPen(*wxRED, 1));
	dc.DrawLine(a, b);

	DrawPanel::DrawPoint(dc, a, wxWHITE);
	DrawPanel::DrawPoint(dc, b, wxWHITE);
}


void LineEdit::Slice_OnMouseLeftDown(wxMouseEvent &e)
{
	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);
	poly->Slice(m_plane);
	
	/* Back to the start */
	m_state = LineEditState_t::START_POINT;
}


void LineEdit::Slice_OnMouseRightDown(wxMouseEvent &e)
{
	m_plane.Flip();

	m_points.clear();

	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);
	poly->ImposePlane(m_plane, m_points);
}


void LineEdit::Slice_OnPaint(wxPaintDC &dc)
{
	std::vector<wxPoint> s_points;
	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);

	const std::vector<wxPoint2DDouble> &points = poly->GetPoints();
	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble point = points[i];
		wxPoint s_point = m_panel->WorldToScreen(point);
		s_points.push_back(s_point);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(*wxBLACK, 3));
	dc.DrawPolygon(s_points.size(), s_points.data());
	dc.SetPen(wxPen(*wxWHITE, 1));
	dc.DrawPolygon(s_points.size(), s_points.data());

	s_points.clear();
	for(wxPoint2DDouble point : m_points) {
		wxPoint s_point = m_panel->WorldToScreen(point);
		s_points.push_back(s_point);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(*wxBLACK, 3));
	dc.DrawPolygon(s_points.size(), s_points.data());
	dc.SetPen(wxPen(*wxGREEN, 1));
	dc.DrawPolygon(s_points.size(), s_points.data());
}


LineEdit::LineEdit(DrawPanel *parent)
	: IBaseEdit(parent)
{
	m_state = LineEditState_t::START_POINT;

	Bind(wxEVT_PAINT, &LineEdit::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &LineEdit::OnMouseLeftDown, this);
	Bind(wxEVT_RIGHT_DOWN, &LineEdit::OnMouseRightDown, this);
}


LineEdit::~LineEdit()
{
}

void LineEdit::OnMouseRightDown(wxMouseEvent &e) 
{
	e.Skip();

	if(m_state == LineEditState_t::SLICE) {
		Slice_OnMouseRightDown(e);
	}
}

void LineEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	switch(m_state)
	{
	case LineEditState_t::START_POINT:
		StartPoint_OnMouseLeftDown(e);
		break;
	case LineEditState_t::END_POINT:
		EndPoint_OnMouseLeftDown(e);
		break;
	case LineEditState_t::SLICE:
		Slice_OnMouseLeftDown(e);
		break;
	}

	e.Skip();
}

void LineEdit::DrawPolygon(wxPaintDC &dc, const ConvexPolygon *p)
{
	if(p == m_context->GetSelectedPoly() && m_state == LineEditState_t::SLICE) {
		Slice_OnPaint(dc);
	} else {
		IBaseEdit::DrawPolygon(dc, p);
	}
}

void LineEdit::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(m_panel);

	switch(m_state)
	{
	case LineEditState_t::START_POINT:
		StartPoint_OnPaint(dc);
		break;
	case LineEditState_t::END_POINT:
		EndPoint_OnPaint(dc);
		break;
	case LineEditState_t::SLICE:
		/* Slice is handled by DrawPolygon */
		break;
	}

	e.Skip();
}
