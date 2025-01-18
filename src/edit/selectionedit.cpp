
#include <cmath>
#include <wx/event.h>

#include "src/drawpanel.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/selectionedit.hpp"

SelectionEdit::SelectionEdit(DrawPanel *panel)
	: IBaseEdit(panel)
{
	Bind(wxEVT_LEFT_DOWN, &SelectionEdit::OnMouseLeftDown, this);
	Bind(wxEVT_MOTION, &SelectionEdit::OnMouseMotion, this);
	Bind(wxEVT_LEFT_UP, &SelectionEdit::OnMouseLeftUp, this);
};

void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint2DDouble world_pos = m_panel->MouseToWorld(e);

	ConvexPolygon *poly = m_context->SelectPoly(world_pos);

	if(poly != nullptr) {
		m_inedit = true;
		m_editstart = world_pos;
		m_context->SetSelectedPoly(poly);
	}

	e.Skip(true);
}


void SelectionEdit::OnMouseMotion(wxMouseEvent &e)
{
	e.Skip(true);

	if(!m_inedit) {
		return;
	}

	wxPoint2DDouble world_pos = m_panel->MouseToWorld(e);
	wxPoint2DDouble delta = world_pos - m_editstart;

	if(m_panel->IsSnapToGrid()) {
		/* Move by at least the size of a grid spacing.
		   Only works if the polygon is already aligned with the grid,
		   TODO: make sure points are aligned to the grid, or AABB is aligned to grid? */
		double spacing = static_cast<double>(m_panel->GetGridSpacing());
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

	ConvexPolygon *selected = m_context->GetSelectedPoly();

	selected->MoveBy(delta);

	bool intersects = false;
	for(ConvexPolygon &poly : m_context->GetPolys()) {
		if(&poly != selected && selected->Intersects(poly)) {
			intersects = true;
			break;
		}
	}

	/* go back */
	selected->MoveBy(-delta);

	if(!intersects) {
		EditAction_Move action;
		action.delta = delta;
		m_context->ApplyAction(action);
	}
}


void SelectionEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	m_inedit = false;
	e.Skip(true);
}
