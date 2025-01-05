#include "src/edit/lineedit.hpp"


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
	m_poly->Slice(m_plane);
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
	m_parent->FinishEdit();
}


void LineEdit::Slice_OnPaint(wxPaintDC &dc)
{
}


LineEdit::LineEdit(DrawPanel *parent)
	: IBaseEdit(parent)
{
	m_state = LineEditState_t::START_POINT;
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
}

void LineEdit::OnPaint(wxPaintDC &dc)
{
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
}