#include "src/edit/lineedit.hpp"
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>


void LineEdit::StartPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble start = m_parent->ScreenToWorld(mpos);
	ConvexPolygon  *poly = m_parent->ClosestPoly(start, k_threshold);

	if(poly == nullptr) {
		m_parent->FinishEdit();
	} else {
		wxPoint2DDouble closest_point;
		if(poly->ClosestPoint(start, k_threshold, closest_point, &m_edge)) {
			m_start = closest_point;
			m_poly = poly;
			m_state = LineEditState_t::END_POINT;
		}
	}
}


void LineEdit::StartPoint_OnPaint(wxPaintDC &dc)
{
	wxPoint2DDouble start = m_parent->GetMousePos();
	ConvexPolygon *poly = m_parent->ClosestPoly(start, k_threshold);

	if(poly != nullptr) {
		wxPoint2DDouble closest_point = start;
		if(poly->ClosestPoint(start, k_threshold, closest_point)) {
			wxPoint spt = m_parent->WorldToScreen(closest_point);
			m_parent->DrawPoint(dc, spt, wxWHITE);
		}
	}
}


void LineEdit::EndPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	m_end = m_parent->MouseToWorld(e);
	m_plane = plane_t(m_start, m_end);

	m_points.clear();
	m_poly->ImposePlane(m_plane, m_points);

	m_state = LineEditState_t::SLICE;
}


void LineEdit::EndPoint_OnPaint(wxPaintDC &dc)
{
	wxPoint a, b;
	wxPoint2DDouble mpos = m_parent->GetMousePos();

	wxPoint2DDouble closest_point;
	if(m_poly->ClosestPoint(mpos, k_threshold, closest_point, nullptr, &m_edge)) {
		a = m_parent->WorldToScreen(m_start);
		b = m_parent->WorldToScreen(closest_point);

		dc.SetPen(wxPen(*wxBLACK, 2));
		dc.DrawLine(a, b);
		dc.SetPen(wxPen(*wxRED, 1));
		dc.DrawLine(a, b);

		DrawPanel::DrawPoint(dc, a, wxWHITE);
		DrawPanel::DrawPoint(dc, a, wxWHITE);
		DrawPanel::DrawPoint(dc, b, wxWHITE);
	}
}


void LineEdit::Slice_OnMouseLeftDown(wxMouseEvent &e)
{
	//m_plane.Flip();
	m_poly->Slice(m_plane);
	/* Back to the start */
	m_state = LineEditState_t::START_POINT;
}


void LineEdit::Slice_OnPaint(wxPaintDC &dc)
{
	std::vector<wxPoint> s_points;

	size_t npoints = m_poly->NumPoints();
	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble point = m_poly->GetPoint(i);
		wxPoint s_point = m_parent->WorldToScreen(point);
		s_points.push_back(s_point);
	}

	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(wxPen(*wxBLACK, 3));
	dc.DrawPolygon(s_points.size(), s_points.data());
	dc.SetPen(wxPen(*wxWHITE, 1));
	dc.DrawPolygon(s_points.size(), s_points.data());

	s_points.clear();
	for(wxPoint2DDouble point : m_points) {
		wxPoint s_point = m_parent->WorldToScreen(point);
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

	Bind(wxEVT_PAINT, &LineEdit::OnPaint, this, wxID_ANY);
	Bind(wxEVT_LEFT_DOWN, &LineEdit::OnMouseLeftDown, this, wxID_ANY);
}


LineEdit::~LineEdit()
{
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
	default:
		break;
	}

	e.Skip(true);
}

void LineEdit::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(m_parent);

	switch(m_state)
	{
	case LineEditState_t::START_POINT:
		StartPoint_OnPaint(dc);
		break;
	case LineEditState_t::END_POINT:
		EndPoint_OnPaint(dc);
		break;
	case LineEditState_t::SLICE:
		Slice_OnPaint(dc);
	default:
		break;
	}

	e.Skip(true);
}
