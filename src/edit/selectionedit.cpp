
#include <cmath>
#include <wx/event.h>

#include "src/glcanvas.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/selectionedit.hpp"

SelectionEdit::SelectionEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	Bind(wxEVT_LEFT_DOWN, &SelectionEdit::OnMouseLeftDown, this);
	Bind(wxEVT_MOTION, &SelectionEdit::OnMouseMotion, this);
	Bind(wxEVT_LEFT_UP, &SelectionEdit::OnMouseLeftUp, this);
};

void SelectionEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	Point2D world_pos = m_view.MouseToWorld(e);

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

	Point2D world_pos = m_view.MouseToWorld(e);
	Point2D delta = world_pos - m_editstart;

	if(m_canvas->IsSnapToGrid()) {
		/* Move by at least the size of a grid spacing.
		   Only works if the polygon is already aligned with the grid,
		   TODO: make sure points are aligned to the grid, or AABB is aligned to grid? */
		double spacing = static_cast<double>(m_canvas->GetGridSpacing());
		if(delta.x * delta.x > spacing * spacing) { 
			delta.x -= fmod(delta.x, spacing); 
			m_editstart.x = world_pos.x; 
		} else { 
			delta.x = 0.0; 
		}
		if(delta.y * delta.y > spacing * spacing) {
			delta.y -= fmod(delta.y, spacing); 
			m_editstart.y = world_pos.y; 
		} else {
			delta.y = 0.0;
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
		m_context->AppendAction(action);
	}

	m_canvas->Refresh(true);
}


void SelectionEdit::OnMouseLeftUp(wxMouseEvent &e)
{
	m_inedit = false;
	e.Skip(true);
}
