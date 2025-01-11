
#ifndef CSELECTIONEDIT_HPP
#define CSELECTIONEDIT_HPP

#include "src/edit/editorcontext.hpp"

class SelectionEdit : public IBaseEdit
{
public:
	SelectionEdit(DrawPanel *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
private:
	bool m_inedit = false;
	wxPoint2DDouble m_editstart;
};

#endif
