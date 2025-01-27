
#ifndef CSELECTIONEDIT_HPP
#define CSELECTIONEDIT_HPP

#include "src/geometry.hpp"
#include "src/edit/editorcontext.hpp"

class SelectionEdit : public IBaseEdit
{
public:
	SelectionEdit(GLCanvas *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
private:
	bool m_inedit = false;
	Point2D m_editstart;
};

#endif
