#ifndef _CRECTANGLEDIT_HPP
#define _CRECTANGEEDIT_HPP

#include <wx/geometry.h>
#include "src/edit/editorcontext.hpp"

class RectangleEdit : public IBaseEdit
{
public:
	RectangleEdit(DrawPanel *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnPaint(wxPaintEvent &e);
private:
	bool m_inedit = false;
	wxRect2DDouble m_tmprect;
	wxPoint2DDouble m_editstart;
};

#endif
