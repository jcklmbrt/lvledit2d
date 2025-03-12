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
	glm::vec2 world_pos = m_view.MouseToWorld(e);

	e.Skip(true);

	EditorLayer *layer = m_context->GetSelectedLayer();
	if(layer == nullptr) {
		return;
	}

	bool intersects = false;

	if(m_inedit) {
		/* 2nd left click */
		for(ConvexPolygon &poly : layer->GetPolys()) {
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
		GLBackgroundGrid::Snap(world_pos);

		for(ConvexPolygon &poly : layer->GetPolys()) {
			if(poly.Contains(world_pos)) {
				intersects = true;
				break;
			}
		}

		if(!intersects) {
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

	glm::vec2 mpos = m_view.MouseToWorld(e);

	GLBackgroundGrid::Snap(mpos);

	if(mpos.x != m_start.x && mpos.y != m_start.y) {
		m_rect.mins = m_start;
		m_rect.maxs = m_start;
		m_onepoint = false;
	} else {
		m_onepoint = true;
		return;
	}

	if(mpos.x > m_start.x) {
		m_rect.maxs.x = mpos.x;
	} else {
		m_rect.mins.x = mpos.x;
	}
	if(mpos.y > m_start.y) {
		m_rect.maxs.y = mpos.y;
	} else {
		m_rect.mins.y = mpos.y;
	}
}

void RectangleEdit::OnDraw()
{
	glm::vec4 color = WHITE;
	glm::vec2 mpos = m_canvas->mousepos;

	EditorLayer *layer = m_context->GetSelectedLayer();
	if(layer == nullptr) {
		return;
	}

	GLBackgroundGrid::Snap(mpos);

	if(m_inedit && !m_onepoint) {
		for(ConvexPolygon &poly : layer->GetPolys()) {
			if(poly.Intersects(m_rect)) {
				color = RED;
			}
		}

		m_canvas->OutlineRect(m_rect, 3.0, BLACK);
		m_canvas->OutlineRect(m_rect, 1.0, color);
	}

	if(!m_inedit) {
		bool intersects = false;
		for(ConvexPolygon &poly : layer->GetPolys()) {
			if(poly.Contains(mpos)) {
				intersects = true;
				break;
			}
		}

		if(intersects) {
			color = RED;
		}
	} else {
		m_canvas->DrawPoint(m_start, color);
	}

	m_canvas->DrawPoint(mpos, color);
}
