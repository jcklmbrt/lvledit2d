
#ifndef _VIEWMATRIX_HPP
#define _VIEWMATRIX_HPP

#include <wx/event.h>
#include "src/singleton.hpp"
#include "src/geometry.hpp"

#define MAX_PAN_X  10000.0
#define MIN_PAN_X -10000.0
#define MAX_PAN_Y  10000.0
#define MIN_PAN_Y -10000.0
#define MAX_ZOOM 10.0
#define MIN_ZOOM 0.01


class ViewMatrixBase : Immobile
{
public:
	ViewMatrixBase();
	void Pan(glm_vec2 point);
	void Zoom(glm_vec2 point, float factor);
	float GetZoom() const { return m_zoom; }
	wxPoint WorldToScreen(glm_vec2 world) const;
	glm_vec2 ScreenToWorld(wxPoint screen) const;
	glm_vec2 MouseToWorld(wxMouseEvent &e) const;
	glm::mat4 GetMatrix() const { return m_view; }
private:
	void SetupMatrix();
	glm::mat4 m_view;
	float m_zoom  = 1.0f;
	glm_vec2 m_pan = { 0.0f, 0.0f };
};


class ViewMatrixCtrl : public wxEvtHandler,
                       public ViewMatrixBase
{
public:
	ViewMatrixCtrl(wxWindow *parent);
private:
	void OnMouseMiddleDown(wxMouseEvent &e);
	void OnMouseMiddleUp(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
private:
	bool m_inpan;
	glm_vec2 m_lastmousepos;
	wxWindow *m_parent;
};


#endif
