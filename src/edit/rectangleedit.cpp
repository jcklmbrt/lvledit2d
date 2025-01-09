#include <wx/event.h>

#include "src/drawpanel.hpp"
#include "src/edit/rectangleedit.hpp"

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

	if(m_panel->IsSnapToGrid()) {
		double spacing = static_cast<double>(m_panel->GetGridSpacing());
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
	wxPoint2DDouble world_pos = m_panel->ScreenToWorld(mpos);

	e.Skip(true);

	if(m_inedit) {
		/* 2nd left click */
		std::vector<ConvexPolygon> &polys = m_context->GetPolys();
		polys.push_back(m_tmprect);
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

	wxPoint2DDouble mpos = m_panel->MouseToWorld(e);

	if(m_panel->IsSnapToGrid()) {
		BackgroundGrid::Snap(mpos);
	}

	if(mpos.m_x > m_editstart.m_x) {
		m_tmprect.SetRight(mpos.m_x);
	}
	else {
		m_tmprect.SetLeft(mpos.m_x);
	}
	if(mpos.m_y > m_editstart.m_y) {
		m_tmprect.SetBottom(mpos.m_y);
	}
	else {
		m_tmprect.SetTop(mpos.m_y);
	}
}

void RectangleEdit::OnPaint(wxPaintEvent &e)
{
	e.Skip(true);

	wxPaintDC dc(m_panel);

	if(!m_inedit) {
		wxPoint2DDouble mpos = m_panel->GetMousePos();
		if(m_panel->IsSnapToGrid()) {
			BackgroundGrid::Snap(mpos);
		}
		wxPoint spos = m_panel->WorldToScreen(mpos);
		DrawPanel::DrawPoint(dc, spos, wxWHITE);
		return;
	}

	wxPen pens[2] = { wxPen(*wxBLACK, 3), wxPen(*wxWHITE, 1) };
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	for(wxPen pen : pens) {
		wxPoint2DDouble lt, rb;
		lt.m_x = m_tmprect.GetLeft();
		lt.m_y = m_tmprect.GetTop();
		rb.m_x = m_tmprect.GetRight();
		rb.m_y = m_tmprect.GetBottom();

		wxPoint slt, srb;
		slt = m_panel->WorldToScreen(lt);
		srb = m_panel->WorldToScreen(rb);

		dc.SetPen(pen);

		wxSize size;
		size.x = srb.x - slt.x;
		size.y = srb.y - slt.y;

		dc.DrawRectangle(slt, size);
	}
}
