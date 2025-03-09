
#ifndef _LINEEDIT_HPP
#define _LINEEDIT_HPP

#include <wx/dcclient.h>
#include <wx/event.h>
#include <wx/geometry.h>
#include <wx/mousestate.h>
#include "src/edit/editorcontext.hpp"

struct ConvexPolygon;

enum class LineEditState 
{
	START_POINT,
	END_POINT,
	SLICE
};

class LineEdit : public IBaseEdit
{
public:
	LineEdit(GLCanvas *parent);
	~LineEdit();
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseRightDown(wxMouseEvent &e);
	void OnDraw();
	void DrawPolygon(const ConvexPolygon *p);
private:
	/* START_POINT */
	void StartPoint_OnMouseLeftDown(wxMouseEvent &e);
	void StartPoint_OnDraw();
	/* END_POINT */
	void EndPoint_OnMouseLeftDown(wxMouseEvent &e);
	void EndPoint_OnDraw();
	/* SLICE */
	void Slice_OnMouseLeftDown(wxMouseEvent &e);
	void Slice_OnMouseRightDown(wxMouseEvent &e);
	void Slice_OnDraw();
public:
	glm::vec2 m_start;
	glm::vec2 m_end;
	Plane2D m_plane;
	std::vector<glm::vec2> m_points;
private:
	LineEditState m_state = LineEditState::START_POINT;
};


#endif
