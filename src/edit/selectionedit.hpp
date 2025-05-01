
#ifndef _SELECTIONEDIT_HPP
#define _SELECTIONEDIT_HPP

#include <wx/event.h>
#include <wx/geometry.h>

#include "src/geometry.hpp"
#include "src/edit/ibaseedit.hpp"

class SelectionEdit : public IBaseEdit
{
public:
	static constexpr int THRESHOLD = 12;
	SelectionEdit(GLCanvas *panel);
	void DrawPolygon(const ConvexPolygon *p) override;
	void OnMouseRightDown(wxMouseEvent &e);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);
private:
	bool m_inedit = false;
	int m_outcode;
	glm::i32vec2 m_delta;
	glm::i32vec2 m_editstart;
};

#endif
