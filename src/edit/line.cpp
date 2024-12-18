
#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <box2d/collision.h>
#include "src/edit/line.hpp"


void LineEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	switch(m_state) {
	case State::SELECT:
		if(!m_parent->SelectPoly(world_pos, m_selectedpoly)) {
			m_parent->FinishEdit();
		} else {
			m_state = State::START_POINT;
		}
		break;
	case State::START_POINT:
		m_editstart = world_pos;
		m_state     = State::END_POINT;
		break;
	case State::END_POINT:
		m_editend = world_pos;
		m_state   = State::CHOP;
		bool ok = true;
		ok &= PolySegment(m_editstart, m_editend,   m_start);
		ok &= PolySegment(m_editend,   m_editstart, m_end);
		if(!ok) m_parent->FinishEdit();
		break;
	}
}

void LineEdit::OnMouseLeftUp(wxMouseEvent &e) {}
void LineEdit::OnMouseMotion(wxMouseEvent &e) {}

bool LineEdit::PolySegment(wxPoint2DDouble start, wxPoint2DDouble end, wxPoint2DDouble &p)
{
	b2Polygon &poly = m_parent->at(m_selectedpoly);
	b2CastOutput out;
	b2RayCastInput in;
	in.maxFraction = 1.0f;
	in.origin.x = static_cast<float>(start.m_x);
	in.origin.y = static_cast<float>(start.m_y);
	wxPoint2DDouble dir = end - start;
	//dir.Normalize();
	in.translation.x = static_cast<float>(dir.m_x);
	in.translation.y = static_cast<float>(dir.m_y);
	
	out = b2RayCastPolygon(&in, &poly);

	if(!out.hit) {
		return false;
	}

	p.m_x = static_cast<double>(out.point.x);
	p.m_y = static_cast<double>(out.point.y);

	return true;
}

void LineEdit::OnPaint(wxPaintDC &dc)
{
	wxPoint a, b;
	wxPoint2DDouble mpos = m_parent->GetMousePos();
	switch(m_state) {
	default:
		break;
	case State::END_POINT:
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		a = m_parent->WorldToScreen(m_editstart);
		b = m_parent->WorldToScreen(mpos);
		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawLine(a, b);
		dc.SetPen(wxPen(*wxWHITE, 1));
		dc.DrawLine(a, b);
		break;
	case State::CHOP:
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		a = m_parent->WorldToScreen(m_start);
		b = m_parent->WorldToScreen(m_end);
		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawLine(a, b);
		dc.SetPen(wxPen(*wxGREEN, 1));
		dc.DrawLine(a, b);

		wxBrush brushes[] = {
			*wxBLACK_BRUSH,
			wxBrush(*wxWHITE),
			*wxBLACK_BRUSH
		};

		int sizes[] = {
			6, 4, 2
		};

		dc.SetPen(*wxTRANSPARENT_PEN);

		// highlight corners
		for(int i = 0; i < 3; i++) {
			dc.SetBrush(brushes[i]);
			int size   = sizes[i];
			int size_2 = size / 2;

			wxRect cr;
			cr.SetWidth(size);
			cr.SetHeight(size);
			cr.SetX(a.x - size_2);
			cr.SetY(a.y - size_2);
			dc.DrawRectangle(cr);

			cr.SetX(b.x - size_2);
			cr.SetY(b.y - size_2);
			dc.DrawRectangle(cr);
		}
		break;
	}
}
