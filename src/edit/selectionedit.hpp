
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
	void OnMove(glm::mat3 &t, const glm_vec2 &delta);
	void OnScale(glm::mat3 &t, const glm_vec2 &delta);
private:
	bool m_inedit = false;
	int m_outcode;
	glm_vec2 m_delta;
	glm_vec2 m_editstart;
};

#endif
