
#ifndef _LINEEDIT_HPP
#define _LINEEDIT_HPP

#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/geometry.h>
#include <wx/mousestate.h>
#include "src/edit/editorcontext.hpp"

class ConvexPolygon;

enum class LineEditState_t 
{
	START_POINT,
	END_POINT,
	SLICE
};

class LineEdit : public IBaseEdit
{
public:
	LineEdit(DrawPanel *parent);
	~LineEdit();
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseRightDown(wxMouseEvent &e);
	void OnPaint(wxPaintEvent &e);
	void DrawPolygon(wxPaintDC &dc, const ConvexPolygon *p);
private:
	/* START_POINT */
	void StartPoint_OnMouseLeftDown(wxMouseEvent &e);
	void StartPoint_OnPaint(wxPaintDC &dc);
	/* END_POINT */
	void EndPoint_OnMouseLeftDown(wxMouseEvent &e);
	void EndPoint_OnPaint(wxPaintDC &dc);
	/* SLICE */
	void Slice_OnMouseLeftDown(wxMouseEvent &e);
	void Slice_OnMouseRightDown(wxMouseEvent &e);
	void Slice_OnPaint(wxPaintDC &dc);
public:
	wxPoint2DDouble m_start;
	wxPoint2DDouble m_end;
	edge_t          m_edge;
	plane_t         m_plane;
	std::vector<wxPoint2DDouble> m_points;
	constexpr static double k_threshold = 1000.0;
private:
	LineEditState_t m_state = LineEditState_t::START_POINT;
};


#endif
