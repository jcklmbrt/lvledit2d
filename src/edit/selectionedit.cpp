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
#include "src/edit/editaction.hpp"

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

	glm::vec2 wpos = m_view.MouseToWorld(e);

	if(!m_edit.GetSelectedPoly()) {
		ConvexPolygon *poly = m_edit.FindPoly(wpos);
		if(poly) {
			m_edit.SetSelectedPoly(poly);
		}
	}

	if(m_edit.GetSelectedPoly()) {
		m_edit.AddDelete();
	}
}

static void GetOutcodeCorner(const Rect2D &r, int oc, glm::i32vec2 &corner, glm::i32vec2 &opposite)
{
	corner = r.mins + (r.GetSize() / 2);
	opposite = corner;

	if(oc & RECT2D_LEFT) {
		corner.x = r.maxs.x;
		opposite.x = r.mins.x;
	}
	else if(oc & RECT2D_RIGHT) {
		corner.x = r.mins.x;
		opposite.x = r.maxs.x;
	}

	if(oc & RECT2D_TOP) {
		corner.y = r.maxs.y;
		opposite.y = r.mins.y;
	}
	else if(oc & RECT2D_BOTTOM) {
		corner.y = r.mins.y;
		opposite.y = r.maxs.y;
	}
}


void SelectionEdit::DrawPolygon(const ConvexPolygon *p)
{
	IBaseEdit::DrawPolygon(p);

	if(p != m_edit.GetSelectedPoly()) {
		return;
	}

	const Rect2D &aabb = p->GetAABB();

	std::array aabbpts = {
		glm::vec2 { aabb.mins.x, aabb.mins.y },
		glm::vec2 { aabb.maxs.x, aabb.mins.y },
		glm::vec2 { aabb.mins.x, aabb.maxs.y },
		glm::vec2 { aabb.maxs.x, aabb.maxs.y }
	};

	int oc;
	bool highlight = true;
	glm::vec4 color = PINK;

	if(!m_inedit) {
		glm::vec2 wpos = m_canvas->GetMousePos();
		oc = aabb.GetOutCode(wpos);

		wxPoint mins = m_view.WorldToScreen(aabb.mins);
		wxPoint maxs = m_view.WorldToScreen(aabb.maxs);
		wxPoint mpos = m_view.WorldToScreen(wpos);

		wxRect hitbox;
		hitbox.SetLeftTop(mins);
		hitbox.SetRightBottom(maxs);
		hitbox.Inflate(THRESHOLD, THRESHOLD);

		if(!hitbox.Contains(mpos)) {
			highlight = false;
		}
	} else {
		oc = m_outcode;
		color = RED;
	}

	int out_x = oc & RECT2D_OUTX;
	int out_y = oc & RECT2D_OUTY;

	glm::i32vec2 corner, opposite;
	GetOutcodeCorner(aabb, oc, corner, opposite);

	if(highlight) {
		if(out_x && !out_y) {
			glm::vec2 a = { opposite.x, aabb.mins.y };
			glm::vec2 b = { opposite.x, aabb.maxs.y };
			m_canvas->DrawLine(a, b, 1.0f, color);
		}
		else if(out_y && !out_x) {
			glm::vec2 a = { aabb.mins.x, opposite.y };
			glm::vec2 b = { aabb.maxs.x, opposite.y };
			m_canvas->DrawLine(a, b, 1.0f, color);
		}
	}

	glm::vec2 center = aabb.mins;
	center += glm::vec2(aabb.GetSize()) / 2.0f;
	if(oc == RECT2D_INSIDE && highlight) {
		m_canvas->DrawPoint(center, color);
	} else {
		m_canvas->DrawPoint(center, YELLOW);
	}

	for(glm::vec2 pt : aabbpts) {
		m_canvas->DrawPoint(pt, WHITE);
	}

	if(out_x && out_y && highlight) {
		m_canvas->DrawPoint(opposite, color);
	}
}


void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	glm::vec2 wpos = m_view.MouseToWorld(e);

	ConvexPolygon *poly = m_edit.FindPoly(wpos);

	if(poly != nullptr) {
		m_editstart = wpos;
		m_inedit = true;
		m_outcode = RECT2D_INSIDE;
		m_edit.SetSelectedPoly(poly);
	} else {
		poly = m_edit.GetSelectedPoly();
		if(poly != nullptr) {
			const Rect2D &aabb = poly->GetAABB();
			m_inedit = true;
			m_outcode = aabb.GetOutCode(wpos);
			m_editstart = GLBackgroundGrid::Snap(wpos);
			glm::i32vec2 opposite;
			
			if(m_outcode != RECT2D_INSIDE) {
				/* too far away */
				wxPoint mins = m_view.WorldToScreen(aabb.mins);
				wxPoint maxs = m_view.WorldToScreen(aabb.maxs);
				wxPoint mpos = e.GetPosition();

				wxRect hitbox;
				hitbox.SetLeftTop(mins);
				hitbox.SetRightBottom(maxs);
				hitbox.Inflate(THRESHOLD, THRESHOLD);

				if(!hitbox.Contains(mpos)) {
					m_inedit = false;
				}

				GetOutcodeCorner(aabb, m_outcode, m_editstart, opposite);
				m_delta = opposite - m_editstart;
			}
		}
	}

	e.Skip(true);
}

void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	EditorLayer *layer = m_edit.GetSelectedLayer();
	if(layer == nullptr) {
		return;
	}

	if(!m_inedit) {
		return;
	}

	glm::i32vec2 wpos = GLBackgroundGrid::Snap(m_view.MouseToWorld(e));

	ConvexPolygon *selected = m_edit.GetSelectedPoly();
	glm::i32vec2 delta = wpos - m_editstart;

	if(m_outcode == RECT2D_INSIDE) {
		glm::i32vec2 scaled_pos = GLBackgroundGrid::Snap(wpos);
		glm::i32vec2 scaled_start = GLBackgroundGrid::Snap(m_editstart);
		delta = scaled_pos - scaled_start;

		m_editstart = wpos;

		selected->Offset(delta);

		bool intersects = false;
		for(ConvexPolygon &poly : layer->GetPolys()) {
			if(&poly != selected && selected->Intersects(poly)) {
				intersects = true;
				break;
			}
		}

		/* go back */
		selected->Offset(-delta);

		if(!intersects) {
			m_edit.AddAction(delta);
		}
	} else {
		Rect2D aabb = selected->GetAABB();
		int outcode = aabb.GetOutCode(wpos);

		if(((outcode ^ m_outcode) & RECT2D_OUTX) == RECT2D_OUTX) {
			m_delta.x = delta.x = 1;
		}

		if(((outcode ^ m_outcode) & RECT2D_OUTY) == RECT2D_OUTY) {
			m_delta.y = delta.y = 1;
		}

		bool out_x = m_outcode & RECT2D_OUTX;
		bool out_y = m_outcode & RECT2D_OUTY;
		if(out_x && !out_y) m_delta.y = delta.y = 1;
		if(out_y && !out_x) m_delta.x = delta.x = 1; 
		
		if(m_delta.x == 0 || m_delta.y == 0) {
			m_inedit = false;
			return;
		}

		if(delta.x == 0 || delta.y == 0 || delta == m_delta) {
			return;
		}

		glm::i32vec2 numer = glm::abs(delta);
		glm::i32vec2 denom = glm::abs(m_delta);

		// normalize fraction
		glm::i32vec2 g;
		g.x = std::gcd(numer.x, denom.x);
		g.y = std::gcd(numer.y, denom.y);
		numer /= g;
		denom /= g;

		// make sure scaling will result in a whole number.
		glm::i32vec2 maxs = ((aabb.maxs - m_editstart) * numer);
		glm::i32vec2 mins = ((aabb.mins - m_editstart) * numer);
		if(maxs.x % denom.x != 0 || maxs.y % denom.y != 0 ||
		   mins.x % denom.x != 0 || mins.y % denom.y != 0) {
			return;
		}

		selected->Scale(m_editstart, numer, denom);

		bool intersects = false;
		for(ConvexPolygon &poly : layer->GetPolys()) {
			if(&poly != selected && selected->Intersects(poly)) {
				intersects = true;
				break;
			}
		}

		selected->Scale(m_editstart, denom, numer);

		if(!intersects) {
			ActScale scale;
			scale.origin = m_editstart;
			scale.denom = denom;
			scale.numer = numer;
			m_edit.AddAction(scale);
			m_delta = delta;
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
		if(m_edit.GetSelectedPoly()) {
			m_edit.AddDelete();
		}
	}
}
