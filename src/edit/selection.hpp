
#ifndef CSELECTIONEDIT_HPP
#define CSELECTIONEDIT_HPP

#include "src/edit/ibaseedit.hpp"

class SelectionEdit : public IBaseEdit
{
public:
	SelectionEdit(DrawPanel *panel)
		: IBaseEdit(panel),
		  m_selectedpoly(-1) {};
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnPaint(wxPaintDC &dc);
private:
	wxPoint2DDouble m_editstart;
	size_t m_selectedpoly;
};

#endif