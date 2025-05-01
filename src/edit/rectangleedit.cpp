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
	glm::i32vec2 world_pos = GLBackgroundGrid::Snap(m_view.MouseToWorld(e));

	e.Skip(true);

	EditorLayer *layer = m_edit.GetSelectedLayer();
	if(layer == nullptr) {
		return;
	}

	bool intersects = false;
	const std::vector<ConvexPolygon> &polys = m_edit.GetPolys();

	if(m_inedit) {
		/* 2nd left click */
		for(size_t i : layer->GetPolys()) {
			const ConvexPolygon &poly = polys[i];
			if(poly.Intersects(m_rect)) {
				intersects = true;
				break;
			}
		}

		if(!intersects && !m_onepoint) {
			m_edit.AddAction(m_rect);
			m_inedit = false;
		}
	} else {
		/* 1st left click*/
		for(size_t i : layer->GetPolys()) {
			const ConvexPolygon &poly = polys[i];
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

	glm::i32vec2 mpos = GLBackgroundGrid::Snap(m_view.MouseToWorld(e));

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
	glm::i32vec2 mpos = GLBackgroundGrid::Snap(m_canvas->GetMousePos());

	std::vector<ConvexPolygon> &polys = m_edit.GetPolys();
	EditorLayer *layer = m_edit.GetSelectedLayer();

	if(layer == nullptr) {
		return;
	}

	if(m_inedit && !m_onepoint) {
		for(size_t i : layer->GetPolys()) {
			ConvexPolygon &poly = polys[i];
			if(poly.Intersects(m_rect)) {
				color = RED;
			}
		}

		m_canvas->OutlineRect(m_rect, 3.0, BLACK);
		m_canvas->OutlineRect(m_rect, 1.0, color);
	}

	if(!m_inedit) {
		bool intersects = false;
		for(size_t i : layer->GetPolys()) {
			ConvexPolygon &poly = polys[i];
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
