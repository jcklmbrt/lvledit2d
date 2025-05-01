#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>

#include "src/geometry.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/lineedit.hpp"


void LineEdit::StartPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	wxPoint mpos = e.GetPosition();
	glm::vec2 start = m_view.ScreenToWorld(mpos);

	if(m_edit.GetSelectedPoly() == nullptr || m_edit.GetSelectedLayer() == nullptr) {
		return;
	}

	m_start = GLBackgroundGrid::Snap(start);
	m_state = LineEditState::END_POINT;
}


void LineEdit::StartPoint_OnDraw()
{
	glm::vec2 mpos = GLBackgroundGrid::Snap(m_canvas->GetMousePos());
	m_canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::EndPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	m_end = GLBackgroundGrid::Snap(m_view.MouseToWorld(e));

	m_plane = Plane2D(m_start, m_end);

	ConvexPolygon *selected = m_edit.GetSelectedPoly();

	if(selected != nullptr) {

		if(selected->AllPointsBehind(m_plane)) {
			/* bad cut, start over. */
			m_state = LineEditState::START_POINT;
		} else {
			m_points.clear();
			selected->ImposePlane(m_plane, m_points);
			m_state = LineEditState::SLICE;
		}
	}
}


void LineEdit::EndPoint_OnDraw()
{
	wxPoint a, b;
	glm::i32vec2 mpos = GLBackgroundGrid::Snap(m_canvas->GetMousePos());

	m_canvas->DrawLine(m_start, mpos, 3.0, BLACK);
	m_canvas->DrawLine(m_start, mpos, 1.0, RED);

	m_canvas->DrawPoint(m_start, WHITE);
	m_canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::Slice_OnMouseLeftDown(wxMouseEvent &e)
{
	wxASSERT(m_edit.GetSelectedLayer() && m_edit.GetSelectedPoly());

	m_edit.AddAction(m_plane);
	/* Back to the start */
	m_state = LineEditState::START_POINT;
}


void LineEdit::Slice_OnMouseRightDown(wxMouseEvent &e)
{
	m_plane.Flip();
	m_points.clear();

	ConvexPolygon *selected = m_edit.GetSelectedPoly();
	wxASSERT(selected);
	wxASSERT(m_edit.GetSelectedLayer());
	selected->ImposePlane(m_plane, m_points);
}


void LineEdit::Slice_OnDraw()
{
	ConvexPolygon *poly = m_edit.GetSelectedPoly();
	wxASSERT(poly);
	wxASSERT(m_edit.GetSelectedLayer());

	const std::vector<glm::vec2> &pts = poly->GetPoints();
	m_canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);
	m_canvas->OutlinePoly(pts.data(), pts.size(), 1.0, RED);

	m_canvas->OutlinePoly(m_points.data(), m_points.size(), 3.0, BLACK);
	m_canvas->OutlinePoly(m_points.data(), m_points.size(), 1.0, GREEN);

	if(poly->GetTexture() != nullptr) {

		Rect2D r;
		r.FitPoints(m_points.data(), m_points.size());
		r = poly->GetUV(r);

		m_canvas->TexturePoly(m_points.data(), m_points.size(), r, *poly->GetTexture(), WHITE);
	}
}


LineEdit::LineEdit(GLCanvas *canvas)
	: IBaseEdit(canvas)
{
	m_state = LineEditState::START_POINT;
	Bind(wxEVT_LEFT_DOWN, &LineEdit::OnMouseLeftDown, this);
	Bind(wxEVT_RIGHT_DOWN, &LineEdit::OnMouseRightDown, this);
}


LineEdit::~LineEdit()
{
}

void LineEdit::OnMouseRightDown(wxMouseEvent &e) 
{
	e.Skip();

	if(m_state == LineEditState::SLICE) {
		Slice_OnMouseRightDown(e);
	}
}

void LineEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	switch(m_state)
	{
	case LineEditState::START_POINT:
		StartPoint_OnMouseLeftDown(e);
		break;
	case LineEditState::END_POINT:
		EndPoint_OnMouseLeftDown(e);
		break;
	case LineEditState::SLICE:
		Slice_OnMouseLeftDown(e);
		break;
	}

	e.Skip();
}

void LineEdit::DrawPolygon(const ConvexPolygon *p)
{
	ConvexPolygon *selected = m_edit.GetSelectedPoly();

	if(p == selected && m_state == LineEditState::SLICE) {
		Slice_OnDraw();
	} else {
		IBaseEdit::DrawPolygon(p);
	}
}

void LineEdit::OnDraw()
{
	switch(m_state)
	{
	case LineEditState::START_POINT:
		StartPoint_OnDraw();
		break;
	case LineEditState::END_POINT:
		EndPoint_OnDraw();
		break;
	case LineEditState::SLICE:
		/* Slice is handled by DrawPolygon */
		break;
	}
}
