#include <array>
#include <cfloat>
#include <cmath>
#include <wx/event.h>

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

	glm_vec2 wpos = view.MouseToWorld(e);

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
		aabb.mins,
		aabb.GetRightTop(),
		aabb.maxs,
		aabb.GetLeftBottom()
	};

	canvas->DrawPoint(aabb.GetCenter(), YELLOW);

	for(glm_vec2 pt : aabbpts) {
		canvas->DrawPoint(pt, WHITE);
	}

	if(m_inedit && m_outcode != RECT2D_INSIDE) {
		canvas->DrawPoint(m_editstart, RED);
	}
}

void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	glm_vec2 wpos = view.MouseToWorld(e);

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
			glm_vec2 opposite;
			
			if(m_outcode != RECT2D_INSIDE) {
				m_editstart = poly->aabb.GetCenter();
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
			}
		}
	}

	e.Skip(true);
}

static glm::mat3 ScaleAroundPoint(const glm_vec2 &p, const glm_vec2 &scale)
{
	return {
		scale.x, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f,
		p.x * (1.0f - scale.x), p.y * (1.0f - scale.y), 1.0f
	};
}
	 
static glm::mat3 Transform(const glm_vec2 &delta)
{
	return {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		delta.x, delta.y, 1.0f
	};
}

void SelectionEdit::OnMove(glm::mat3 &t, const glm_vec2 &wpos)
{
	glm_vec2 delta = wpos - m_editstart;

	if(context->snaptogrid) {
		glm_vec2 scaled_pos = wpos;
		glm_vec2 scaled_start = m_editstart;
		GLBackgroundGrid::Snap(scaled_pos);
		GLBackgroundGrid::Snap(scaled_start);
		delta = scaled_pos - scaled_start;
	}

	m_editstart = wpos;

	t = Transform(delta);
}

void SelectionEdit::OnScale(glm::mat3 &t, const glm_vec2 &wpos)
{
	glm_vec2 delta = wpos - m_editstart;

	if(context->snaptogrid) {
		GLBackgroundGrid::Snap(delta);
		GLBackgroundGrid::Snap(m_delta);
	}

	glm_vec2 scale = delta / m_delta;

	bool out_x = m_outcode & (RECT2D_LEFT | RECT2D_RIGHT);
	bool out_y = m_outcode & (RECT2D_TOP | RECT2D_BOTTOM);
	if(out_x && !out_y) scale.y = 1.0f;
	if(out_y && !out_x) scale.x = 1.0f; 
	
	t = ScaleAroundPoint(m_editstart, scale);

	m_delta = delta;
}

void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	if(!m_inedit) {
		return;
	}

	glm_vec2 wpos = view.MouseToWorld(e);

	ConvexPolygon *selected = context->GetSelectedPoly();

	glm::mat3 t;
	if(m_outcode == RECT2D_INSIDE) {
		OnMove(t, wpos);
	} else {
		/* phase 1: check if the mouse is on the other side */
		int outcode = selected->aabb.GetOutCode(wpos);
		if(outcode & m_outcode || outcode == RECT2D_INSIDE) {
			OnScale(t, wpos);
		}

		/* phase 2: check if the transformation will preserve orientation */
		glm::mat2 linear = glm::mat2(t);
		float det = glm::determinant(linear);
		if(det <= 0) {
			m_inedit = false;
			return;
		}

		/* phase 3: check if the transformation will invert the bounding box */
		glm_vec2 new_mins = t * glm::vec3(selected->aabb.mins, 1.0f);
		glm_vec2 new_maxs = t * glm::vec3(selected->aabb.maxs, 1.0f);

		if(new_mins.x >= new_maxs.x || new_mins.y >= new_maxs.y) {
			m_inedit = false;
			return;
		}
	}

	selected->Transform(t);

        bool intersects = false;
	for(ConvexPolygon &poly : context->polys) {
		if(&poly != selected && selected->Intersects(poly)) {
			intersects = true;
			break;
		}
	}

	/* go back */
	selected->Transform(inverse(t));

	if(!intersects) {
		EditAction_Trans act;
		act.matrix = t;
		context->AppendAction(act);
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
