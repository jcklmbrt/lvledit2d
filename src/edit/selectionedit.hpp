
#ifndef CSELECTIONEDIT_HPP
#define CSELECTIONEDIT_HPP

#include "src/edit/ibaseedit.hpp"

class SelectionEdit : public IBaseEdit
{
public:
	SelectionEdit(DrawPanel *panel)
		: IBaseEdit(panel),
		  m_poly(nullptr) {};
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnPaint(wxPaintDC &dc);
private:
	wxPoint2DDouble m_editstart;
	ConvexPolygon  *m_poly;
};

#endif