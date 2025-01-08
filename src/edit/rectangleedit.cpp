
#include "src/edit/rectangleedit.hpp"
#include <wx/event.h>

RectangleEdit::RectangleEdit(DrawPanel *panel)
	: IBaseEdit(panel)
{
	Bind(wxEVT_PAINT, &RectangleEdit::OnPaint, this, wxID_ANY);
	Bind(wxEVT_LEFT_DOWN, &RectangleEdit::OnMouseLeftDown, this, wxID_ANY);
	Bind(wxEVT_MOTION, &RectangleEdit::OnMouseMotion, this, wxID_ANY);
}

void RectangleEdit::StartEdit(wxPoint2DDouble wpos)
{
	m_editstart = wpos;

	if(m_parent->IsSnapToGrid()) {
		double spacing = static_cast<double>(m_parent->GetGridSpacing());
		m_editstart.m_x -= fmodl(m_editstart.m_x, spacing);
		m_editstart.m_y -= fmodl(m_editstart.m_y, spacing);
	}

	m_tmprect = wxRect2DDouble();
	m_tmprect.SetCentre(m_editstart);
	m_inedit = true;
}


void RectangleEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	e.Skip(true);

	if(m_inedit) {
		/* 2nd left click */
		m_parent->push_back(m_tmprect);
		m_inedit = false;
	} else {
		/* 1st left click*/
		StartEdit(world_pos);
		m_inedit = true;
	}
}


void RectangleEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);
	if(!m_inedit) {
		return;
	}

	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	double x = world_pos.m_x;
	double y = world_pos.m_y;

	if(m_parent->IsSnapToGrid()) {
		double spacing = static_cast<double>(m_parent->GetGridSpacing());
		x -= fmodl(x, spacing);
		y -= fmodl(y, spacing);
	}

	if(x > m_editstart.m_x) {
		m_tmprect.SetRight(x);
	}
	else {
		m_tmprect.SetLeft(x);
	}
	if(y > m_editstart.m_y) {
		m_tmprect.SetBottom(y);
	}
	else {
		m_tmprect.SetTop(y);
	}
}

void RectangleEdit::OnPaint(wxPaintEvent &e)
{
	e.Skip(true);
	if(!m_inedit) {
		return;
	}

	wxPaintDC dc(m_parent);

	wxPen pens[2] = { wxPen(*wxBLACK, 3), wxPen(*wxWHITE, 1) };
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	for(wxPen pen : pens) {
		wxPoint2DDouble lt, rb;
		lt.m_x = m_tmprect.GetLeft();
		lt.m_y = m_tmprect.GetTop();
		rb.m_x = m_tmprect.GetRight();
		rb.m_y = m_tmprect.GetBottom();

		wxPoint slt, srb;
		slt = m_parent->WorldToScreen(lt);
		srb = m_parent->WorldToScreen(rb);

		dc.SetPen(pen);

		wxSize size;
		size.x = srb.x - slt.x;
		size.y = srb.y - slt.y;

		dc.DrawRectangle(slt, size);
	}
}
