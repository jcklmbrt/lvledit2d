
#include <cmath>
#include <wx/event.h>
#include "src/edit/selectionedit.hpp"

SelectionEdit::SelectionEdit(DrawPanel *panel)
	: IBaseEdit(panel),
	  m_poly(nullptr)
{
	Bind(wxEVT_LEFT_DOWN, &SelectionEdit::OnMouseLeftDown, this, wxID_ANY);
	Bind(wxEVT_MOTION, &SelectionEdit::OnMouseMotion, this, wxID_ANY);
	Bind(wxEVT_LEFT_UP, &SelectionEdit::OnMouseLeftUp, this, wxID_ANY);
};

void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint2DDouble world_pos = m_parent->MouseToWorld(e);

	m_poly = m_parent->SelectPoly(world_pos);

	if(m_poly != nullptr) {
		m_editstart = world_pos;
	}

	e.Skip(true);
}


void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	if(m_poly == nullptr) {
		return;
	}

	wxPoint2DDouble world_pos = m_parent->MouseToWorld(e);
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

	m_poly->MoveBy(delta);
}


void SelectionEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	m_poly = nullptr;
	e.Skip(true);
}
