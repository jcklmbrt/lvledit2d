
#ifndef _LINEEDIT_HPP
#define _LINEEDIT_HPP

#include <wx/dcclient.h>
#include <wx/geometry.h>
#include <wx/mousestate.h>
#include <box2d/collision.h>
#include "src/edit/ibaseedit.hpp"


class LineEdit : public IBaseEdit
{
	enum class State
	{
		SELECT,
		START_POINT,
		END_POINT,
		CHOP
	} m_state;
public:
	LineEdit(DrawPanel *panel)
		: IBaseEdit(panel),
	          m_state(State::SELECT) {}
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnPaint(wxPaintDC &dc);
private:
	bool PolySegment(wxPoint2DDouble start, wxPoint2DDouble end, wxPoint2DDouble &p);
	wxPoint2DDouble m_editstart;
	wxPoint2DDouble m_editend;
	wxPoint2DDouble m_start;
	wxPoint2DDouble m_end;

	b2Polygon m_poly;

	size_t m_selectedpoly;
};

#endif
