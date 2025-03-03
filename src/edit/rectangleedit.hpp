#ifndef _CRECTANGLEDIT_HPP
#define _CRECTANGEEDIT_HPP

#include "src/geometry.hpp"
#include "src/edit/editorcontext.hpp"

class RectangleEdit : public IBaseEdit
{
public:
	RectangleEdit(GLCanvas *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnDraw();

	bool m_inedit = false;
	bool m_onepoint = false;
	glm_vec2 m_start;
	Rect2D m_rect;
};

#endif
