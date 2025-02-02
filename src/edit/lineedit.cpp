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
	Point2D start = m_view.ScreenToWorld(mpos);
	ConvexPolygon  *poly = m_context->GetSelectedPoly();

	if(poly == nullptr) {
		
	} else {
		if(m_canvas->IsSnapToGrid()) {
			GLBackgroundGrid::Snap(start);
		}
		m_start = start;
		m_context->SetSelectedPoly(poly);
		m_state = LineEditState_t::END_POINT;
	}
}


void LineEdit::StartPoint_OnDraw()
{
	Point2D mpos = m_canvas->GetMousePos();
	if(m_canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(mpos);
	}

	m_canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::EndPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	m_end = m_view.MouseToWorld(e);

	if(m_canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(m_end);
	}

	m_plane = Plane2D(m_start, m_end);

	ConvexPolygon *poly = m_context->GetSelectedPoly();

	if(poly != nullptr) {
		if(poly->AllPointsBehind(m_plane)) {
			/* bad cut, start over. */
			m_state = LineEditState_t::START_POINT;
		} else {
			m_points.clear();
			poly->ImposePlane(m_plane, m_points);
			m_state = LineEditState_t::SLICE;
		}
	}
}


void LineEdit::EndPoint_OnDraw()
{
	wxPoint a, b;
	Point2D mpos = m_canvas->GetMousePos();

	if(m_canvas->IsSnapToGrid()) {
		GLBackgroundGrid::Snap(mpos);
	}

	m_canvas->DrawLine(m_start, mpos, 3.0, BLACK);
	m_canvas->DrawLine(m_start, mpos, 1.0, RED);

	m_canvas->DrawPoint(m_start, WHITE);
	m_canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::Slice_OnMouseLeftDown(wxMouseEvent &e)
{
	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);

	EditAction_Line action;
	action.aabb = poly->GetAABB();
	action.plane = m_plane;
	action.start = m_start;
	action.end = m_end;

	m_context->AppendAction(action);
	
	/* Back to the start */
	m_state = LineEditState_t::START_POINT;
}


void LineEdit::Slice_OnMouseRightDown(wxMouseEvent &e)
{
	m_plane.Flip();

	m_points.clear();

	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);
	poly->ImposePlane(m_plane, m_points);
}


void LineEdit::Slice_OnDraw()
{
	ConvexPolygon *poly = m_context->GetSelectedPoly();
	wxASSERT(poly != nullptr);

	const std::vector<Point2D> &pts = poly->GetPoints();
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
	m_state = LineEditState_t::START_POINT;
	Bind(wxEVT_LEFT_DOWN, &LineEdit::OnMouseLeftDown, this);
	Bind(wxEVT_RIGHT_DOWN, &LineEdit::OnMouseRightDown, this);
}


LineEdit::~LineEdit()
{
}

void LineEdit::OnMouseRightDown(wxMouseEvent &e) 
{
	e.Skip();

	if(m_state == LineEditState_t::SLICE) {
		Slice_OnMouseRightDown(e);
	}
}

void LineEdit::OnMouseLeftDown(wxMouseEvent &e)
{
	switch(m_state)
	{
	case LineEditState_t::START_POINT:
		StartPoint_OnMouseLeftDown(e);
		break;
	case LineEditState_t::END_POINT:
		EndPoint_OnMouseLeftDown(e);
		break;
	case LineEditState_t::SLICE:
		Slice_OnMouseLeftDown(e);
		break;
	}

	e.Skip();
}

void LineEdit::DrawPolygon(const ConvexPolygon *p)
{
	if(p == m_context->GetSelectedPoly() && m_state == LineEditState_t::SLICE) {
		Slice_OnDraw();
	} else {
		IBaseEdit::DrawPolygon(p);
	}
}

void LineEdit::OnDraw()
{
	switch(m_state)
	{
	case LineEditState_t::START_POINT:
		StartPoint_OnDraw();
		break;
	case LineEditState_t::END_POINT:
		EndPoint_OnDraw();
		break;
	case LineEditState_t::SLICE:
		/* Slice is handled by DrawPolygon */
		break;
	}
}
