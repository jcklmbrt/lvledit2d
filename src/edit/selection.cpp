
#include "src/edit/selection.hpp"


void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	wxPoint2DDouble world_pos = m_parent->ScreenToWorld(mpos);

	size_t num_polys = m_parent->size();
	for(size_t i = 0; i < num_polys; i++) {
		ConvexPolygon &p = m_parent->at(i);
		if(p.ContainsPoint(world_pos)) {
			m_selectedpoly = i;
			m_editstart = world_pos;
			return;
		}
	}

	m_parent->FinishEdit();
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
			delta.m_x -= fmodl(delta.m_x, spacing); 
			m_editstart.m_x = world_pos.m_x; 
		} else { 
			delta.m_x = 0.0; 
		}
		if(delta.m_y * delta.m_y > spacing * spacing) {
			delta.m_y -= fmodl(delta.m_y, spacing); 
			m_editstart.m_y = world_pos.m_y; 
		} else {
			delta.m_y = 0.0;
		}
	} else {
		m_editstart = world_pos;
	}

	ConvexPolygon &selected = m_parent->at(m_selectedpoly);
	selected.MoveBy(delta);
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