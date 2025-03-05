#include <array>
#include <cfloat>
#include <cmath>
#include <numeric>
#include <wx/event.h>

#include "glm/fwd.hpp"
#include "src/geometry.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/selectionedit.hpp"

SelectionEdit::SelectionEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	Bind(wxEVT_LEFT_DOWN, &SelectionEdit::OnMouseLeftDown, this);
	Bind(wxEVT_MOTION, &SelectionEdit::OnMouseMotion, this);
	Bind(wxEVT_LEFT_UP, &SelectionEdit::OnMouseLeftUp, this);
	Bind(wxEVT_RIGHT_DOWN, &SelectionEdit::OnMouseRightDown, this);
	Bind(wxEVT_KEY_DOWN, &SelectionEdit::OnKeyDown, this);
}

void SelectionEdit::OnMouseRightDown(wxMouseEvent &e)
{
	e.Skip();

	glm::vec2 wpos = view.MouseToWorld(e);

	if(!context->GetSelectedPoly()) {
		ConvexPolygon *poly = context->FindPoly(wpos);
		if(poly) {
			context->SetSelectedPoly(poly);
		}
	}

	if(context->GetSelectedPoly()) {
		EditAction_Delete action;
		context->AppendAction(action);
	}
}

void SelectionEdit::DrawPolygon(const ConvexPolygon *p)
{
	IBaseEdit::DrawPolygon(p);

	if(p != context->GetSelectedPoly()) {
		return;
	}

	Rect2D aabb = p->aabb;

	std::array aabbpts = {
		glm::vec2 { aabb.mins.x, aabb.mins.y },
		glm::vec2 { aabb.maxs.x, aabb.mins.y },
		glm::vec2 { aabb.mins.x, aabb.maxs.y },
		glm::vec2 { aabb.maxs.x, aabb.maxs.y }
	};

	glm::vec2 center = aabb.mins;
	center += glm::vec2(aabb.GetSize()) / 2.0f;
	canvas->DrawPoint(center, YELLOW);

	for(glm::vec2 pt : aabbpts) {
		canvas->DrawPoint(pt, WHITE);
	}

	if(m_inedit && m_outcode != RECT2D_INSIDE) {
		canvas->DrawPoint(m_editstart, RED);
	}
}

void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	glm::vec2 wpos = view.MouseToWorld(e);

	ConvexPolygon *poly = context->FindPoly(wpos);

	if(poly != nullptr) {
		m_editstart = wpos;
		m_inedit = true;
		m_outcode = RECT2D_INSIDE;
		context->SetSelectedPoly(poly);
	} else {
		poly = context->GetSelectedPoly();
		if(poly != nullptr) {
			Rect2D &aabb = poly->aabb;
			m_inedit = true;
			m_outcode = aabb.GetOutCode(wpos);
			m_editstart = wpos;
			glm::i32vec2 opposite;
			
			if(m_outcode != RECT2D_INSIDE) {
				m_editstart = poly->aabb.mins + (poly->aabb.GetSize() / 2);
				opposite = m_editstart;
				if(m_outcode & RECT2D_LEFT) {
					m_editstart.x = aabb.maxs.x;
					opposite.x = aabb.mins.x;
				} else if(m_outcode & RECT2D_RIGHT) {
					m_editstart.x = aabb.mins.x;
					opposite.x = aabb.maxs.x;
				}
				if(m_outcode & RECT2D_TOP) {
					m_editstart.y = aabb.maxs.y;
					opposite.y = aabb.mins.y;
				} else if(m_outcode & RECT2D_BOTTOM) {
					m_editstart.y = aabb.mins.y;
					opposite.y = aabb.maxs.y;
				}

				m_delta = opposite - m_editstart;
				if(m_delta.x == 0 && m_delta.y == 0) {
					m_inedit = false;
				}
			}
		}
	}

	e.Skip(true);
}

void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	if(!m_inedit) {
		return;
	}

	glm::i32vec2 wpos = view.MouseToWorld(e);

	ConvexPolygon *selected = context->GetSelectedPoly();
	glm::i32vec2 delta = wpos - m_editstart;

	if(m_outcode == RECT2D_INSIDE) {

		if(context->snaptogrid) {
			glm::vec2 scaled_pos = wpos;
			glm::vec2 scaled_start = m_editstart;
			GLBackgroundGrid::Snap(scaled_pos);
			GLBackgroundGrid::Snap(scaled_start);
			delta = scaled_pos - scaled_start;
		}

		m_editstart = wpos;

		selected->Offset(delta);

		bool intersects = false;
		for(ConvexPolygon &poly : context->polys) {
			if(&poly != selected && selected->Intersects(poly)) {
				intersects = true;
				break;
			}
		}

		/* go back */
		selected->Offset(-delta);

		if(!intersects) {
			EditAction_Move act;
			act.delta = delta;
			context->AppendAction(act);
		}
	} else {
		bool out_x = m_outcode & (RECT2D_LEFT | RECT2D_RIGHT);
		bool out_y = m_outcode & (RECT2D_TOP | RECT2D_BOTTOM);
		if(out_x && !out_y) m_delta.y = delta.y = 1;
		if(out_y && !out_x) m_delta.x = delta.x = 1; 

		if(m_delta.x == 0 || m_delta.y == 0) {
			m_inedit = false;
			return;
		}

		glm::i32vec2 numer = glm::abs(delta);
		glm::i32vec2 denom = glm::abs(m_delta);

		glm::i32vec2 g;
		g.x = std::gcd(numer.x, denom.x);
		g.y = std::gcd(numer.y, denom.y);
		numer /= g;
		denom /= g;

		/*
		selected->Scale(m_editstart, numer, denom);
		*/

		bool intersects = false;
		/*
		for(ConvexPolygon &poly : context->polys) {
			if(&poly != selected && selected->Intersects(poly)) {
				intersects = true;
				break;
			}
		}

		selected->Scale(m_editstart, denom, numer);
		*/

		m_delta = delta;

		if(!intersects) {
			EditAction_Scale act;
			act.origin = m_editstart;
			act.denom = denom;
			act.numer = numer;
			context->AppendAction(act);
		}
	}
}


void SelectionEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	m_inedit = false;
	e.Skip(true);
}


void SelectionEdit::OnKeyDown(wxKeyEvent &e)
{
	e.Skip(true);

	if(e.GetKeyCode() == WXK_DELETE || e.GetKeyCode() == WXK_UP) {
		if(context->GetSelectedPoly()) {
			EditAction_Delete action;
			context->AppendAction(action);
		}
	}
}
