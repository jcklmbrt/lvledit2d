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
	Point2D start = view.ScreenToWorld(mpos);

	ConvexPolygon *poly = context->GetSelectedPoly();

	if(poly == nullptr) {
		
	} else {
		if(canvas->editor.snaptogrid) {
			GLBackgroundGrid::Snap(start);
		}
		m_start = start;
		context->SetSelectedPoly(poly);
		m_state = LineEditState_t::END_POINT;
	}
}


void LineEdit::StartPoint_OnDraw()
{
	Point2D mpos = canvas->mousepos;
	if(canvas->editor.snaptogrid) {
		GLBackgroundGrid::Snap(mpos);
	}

	canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::EndPoint_OnMouseLeftDown(wxMouseEvent &e)
{
	m_end = view.MouseToWorld(e);

	if(canvas->editor.snaptogrid) {
		GLBackgroundGrid::Snap(m_end);
	}

	m_plane = Plane2D(m_start, m_end);

	ConvexPolygon *poly = context->GetSelectedPoly();

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
	Point2D mpos = canvas->mousepos;

	if(canvas->editor.snaptogrid) {
		GLBackgroundGrid::Snap(mpos);
	}

	canvas->DrawLine(m_start, mpos, 3.0, BLACK);
	canvas->DrawLine(m_start, mpos, 1.0, RED);

	canvas->DrawPoint(m_start, WHITE);
	canvas->DrawPoint(mpos, WHITE);
}


void LineEdit::Slice_OnMouseLeftDown(wxMouseEvent &e)
{
	ConvexPolygon *poly = context->GetSelectedPoly();
	wxASSERT(poly != nullptr);

	EditAction_Line action;
	action.aabb = poly->aabb;
	action.plane = m_plane;
	action.start = m_start;
	action.end = m_end;

	context->AppendAction(action);
	
	/* Back to the start */
	m_state = LineEditState_t::START_POINT;
}


void LineEdit::Slice_OnMouseRightDown(wxMouseEvent &e)
{
	m_plane.Flip();

	m_points.clear();

	ConvexPolygon *poly = context->GetSelectedPoly();
	wxASSERT(poly != nullptr);
	poly->ImposePlane(m_plane, m_points);
}


void LineEdit::Slice_OnDraw()
{
	ConvexPolygon *poly = context->GetSelectedPoly();
	wxASSERT(poly != nullptr);

	size_t npts = poly->points.size();
	Point2D *pts = poly->points.data();
	canvas->OutlinePoly(pts, npts, 3.0, BLACK);
	canvas->OutlinePoly(pts, npts, 1.0, RED);

	canvas->OutlinePoly(m_points.data(), m_points.size(), 3.0, BLACK);
	canvas->OutlinePoly(m_points.data(), m_points.size(), 1.0, GREEN);

	if(poly->GetTexture() != nullptr) {

		Rect2D r;
		r.FitPoints(m_points.data(), m_points.size());
		r = poly->GetUV(r);

		canvas->TexturePoly(m_points.data(), m_points.size(), r, *poly->GetTexture(), WHITE);
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
	if(p == context->GetSelectedPoly() && m_state == LineEditState_t::SLICE) {
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
