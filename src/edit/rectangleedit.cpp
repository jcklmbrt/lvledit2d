#include <wx/event.h>

#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
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
	Point2D world_pos = View.MouseToWorld(e);

	e.Skip(true);

	bool intersects = false;

	if(m_inedit) {
		/* 2nd left click */
		for(ConvexPolygon &poly : Context->Polygons) {
			if(poly.Intersects(m_rect)) {
				intersects = true;
				break;
			}
		}

		if(!intersects && !m_onepoint) {
			EditAction_Rect action;
			action.rect = m_rect;
			Context->AppendAction(action);
			m_inedit = false;
		}
	} else {
		/* 1st left click*/
		for(ConvexPolygon &poly : Context->Polygons) {
			if(poly.Contains(world_pos)) {
				intersects = true;
				break;
			}
		}

		if(!intersects) {
			if(Canvas->IsSnapToGrid()) {
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

	Point2D mpos = View.MouseToWorld(e);

	if(Canvas->IsSnapToGrid()) {
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
	} else {
		m_rect.SetLeft(mpos.x);
	}
	if(mpos.y > m_start.y) {
		m_rect.SetBottom(mpos.y);
	} else {
		m_rect.SetTop(mpos.y);
	}
}

void RectangleEdit::OnDraw()
{
	Color color;

	Point2D mpos = Canvas->GetMousePos();

	if(Canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(mpos);
	}

	if(m_inedit && !m_onepoint) {

		color = WHITE;

		for(ConvexPolygon &poly : Context->Polygons) {
			if(poly.Intersects(m_rect)) {
				color = RED;
			}
		}

		Canvas->OutlineRect(m_rect, 3.0, BLACK);
		Canvas->OutlineRect(m_rect, 1.0, color);
	}

	bool intersects = false;
	for(ConvexPolygon &poly : Context->Polygons) {
		if(poly.Contains(mpos)) {
			intersects = true;
			break;
		}
	}

	color = WHITE;
	if(intersects || m_onepoint) {
		color = RED;
	}

	Canvas->DrawPoint(mpos, color);
	Canvas->DrawPoint(m_start, color);
}
