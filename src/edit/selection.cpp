#include <box2d/collision.h>
#include <cmath>
#include "box2d/math_functions.h"
#include "src/edit/selection.hpp"


void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	if(m_parent->SelectPoly(world_pos, m_selectedpoly)) {
		/* start edit */
		m_editstart = world_pos;
	} else {
		/* don't bother */
		m_parent->FinishEdit();
	}
}


void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	wxPoint2DDouble delta = world_pos - m_editstart;

	if(m_parent->IsSnapToGrid()) {
		/* Move by at least the size of a grid spacing.
		   Only works if the polygon is already aligned with the grid,
		   TODO: make sure points are aligned to the grid, or AABB is aligned to grid? */
		double spacing = static_cast<double>(m_parent->GetGridSpacing());
		if(delta.m_x * delta.m_x > spacing * spacing) { 
			delta.m_x -= fmod(delta.m_x, spacing); 
			m_editstart.m_x = world_pos.m_x; 
		} else { 
			delta.m_x = 0.0; 
		}
		if(delta.m_y * delta.m_y > spacing * spacing) {
			delta.m_y -= fmod(delta.m_y, spacing); 
			m_editstart.m_y = world_pos.m_y; 
		} else {
			delta.m_y = 0.0;
		}
	} else {
		m_editstart = world_pos;
	}

	b2Vec2 b2delta;
	b2delta.x = static_cast<float>(delta.m_x);
	b2delta.y = static_cast<float>(delta.m_y);
	b2Transform trans = { b2delta, b2Rot_identity };

	b2Polygon &selected = m_parent->at(m_selectedpoly);
	selected = b2TransformPolygon(trans, &selected);
}


void SelectionEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	/* kill me */
	m_parent->FinishEdit();
}


void SelectionEdit::OnPaint(wxPaintDC &dc)
{
	/* noop */
}
