#ifndef _CRECTANGLEDIT_HPP
#define _CRECTANGEEDIT_HPP

#include <wx/geometry.h>
#include "src/edit/ibaseedit.hpp"

class RectangleEdit : public IBaseEdit
{
public:
	RectangleEdit(DrawPanel *panel) 
		: IBaseEdit(panel) {};
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnPaint(wxPaintDC &dc);
private:
	void StartEdit(wxPoint2DDouble wpos);
	bool m_inedit = false;
	wxRect2DDouble  m_tmprect;
	wxPoint2DDouble m_editstart;
};

#endif