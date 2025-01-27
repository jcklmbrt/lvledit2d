#include <wx/event.h>

#include "src/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/rectangleedit.hpp"

RectangleEdit::RectangleEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	Bind(wxEVT_LEFT_DOWN, &RectangleEdit::OnMouseLeftDown, this, wxID_ANY);
	Bind(wxEVT_MOTION, &RectangleEdit::OnMouseMotion, this, wxID_ANY);
}


void RectangleEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	Point2D world_pos = m_view.MouseToWorld(e);

	e.Skip(true);

	std::vector<ConvexPolygon> &polys = m_context->GetPolys();
	bool intersects = false;

	if(m_inedit) {
		/* 2nd left click */
		for(ConvexPolygon &poly : polys) {
			if(poly.Intersects(m_rect)) {
				intersects = true;
				break;
			}
		}

		if(!intersects && !m_onepoint) {
			EditAction_Rect action;
			action.rect = m_rect;
			m_context->AppendAction(action);
			m_inedit = false;
		}
	} else {
		/* 1st left click*/
		for(ConvexPolygon &poly : polys) {
			if(poly.Contains(world_pos)) {
				intersects = true;
				break;
			}
		}

		if(!intersects) {
			if(m_canvas->IsSnapToGrid()) {
				GLBackgroundGrid::Snap(world_pos);
			}

			m_start = world_pos;
			m_inedit = true;
			m_onepoint = true;
		}

	}
}


void RectangleEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);
	if(!m_inedit) {
		return;
	}

	Point2D mpos = m_view.MouseToWorld(e);

	if(m_canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(mpos);
	}

	if(mpos.x != m_start.x && mpos.y != m_start.y) {
		m_rect.SetMins(m_start);
		m_rect.SetMaxs(m_start);
		m_onepoint = false;
	} else {
		m_onepoint = true;
		return;
	}

	if(mpos.x > m_start.x) {
		m_rect.SetRight(mpos.x);
	}
	else {
		m_rect.SetLeft(mpos.x);
	}
	if(mpos.y > m_start.y) {
		m_rect.SetBottom(mpos.y);
	}
	else {
		m_rect.SetTop(mpos.y);
	}
}

void RectangleEdit::OnDraw()
{
	const Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color red = Color(1.0f, 0.0f, 0.0f, 1.0f);
	Color color;

	Point2D mpos = m_canvas->GetMousePos();

	if(m_canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(mpos);
	}

	std::vector<ConvexPolygon> &polys = m_context->GetPolys();

	if(m_inedit && !m_onepoint) {

		color = white;

		for(ConvexPolygon &poly : polys) {
			if(poly.Intersects(m_rect)) {
				color = red;
			}
		}

		m_canvas->DrawRect(m_rect, color);
	}

	bool intersects = false;
	for(ConvexPolygon &poly : polys) {
		if(poly.Contains(mpos)) {
			intersects = true;
			break;
		}
	}

	color = white;
	if(intersects || m_onepoint) {
		color = red;
	}

	m_canvas->DrawPoint(mpos, color);
	m_canvas->DrawPoint(m_start, color);
}
