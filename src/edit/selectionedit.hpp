
#ifndef CSELECTIONEDIT_HPP
#define CSELECTIONEDIT_HPP

#include "src/geometry.hpp"
#include "src/edit/editorcontext.hpp"
#include <wx/event.h>
#include <wx/geometry.h>

class SelectionEdit : public IBaseEdit
{
public:
	SelectionEdit(GLCanvas *panel);
	void DrawPolygon(const ConvexPolygon *p) override;
	void OnMouseRightDown(wxMouseEvent &e);
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);
	void OnMove(Matrix3 &t, const Point2D &delta);
	void OnScale(Matrix3 &t, const Point2D &delta);
private:
	bool m_inedit = false;
	int m_outcode;
	Point2D m_delta;
	Point2D m_editstart;
};

#endif
