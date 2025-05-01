#ifndef _RECTANGLEDIT_HPP
#define _RECTANGLEDIT_HPP

#include "src/geometry.hpp"
#include "src/edit/ibaseedit.hpp"

class RectangleEdit : public IBaseEdit
{
public:
	RectangleEdit(GLCanvas *panel);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnDraw();

	bool m_inedit = false;
	bool m_onepoint = false;
	glm::i32vec2 m_start;
	Rect2D m_rect;
};

#endif
