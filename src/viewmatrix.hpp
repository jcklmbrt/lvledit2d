
#ifndef _VIEWMATRIX_HPP
#define _VIEWMATRIX_HPP

#include <wx/event.h>
#include "src/util/singleton.hpp"
#include "src/geometry.hpp"

constexpr glm::vec2 MAX_PAN = {  1000.0f,  1000.0f };
constexpr glm::vec2 MIN_PAN = { -1000.0f, -1000.0f };
constexpr float MAX_ZOOM = 150.0f;
constexpr float MIN_ZOOM = 5.0f;


class ViewMatrixBase : Immobile
{
public:
	ViewMatrixBase();
	void Pan(glm::vec2 point);
	void Zoom(glm::vec2 point, float factor);
	float GetZoom() const { return m_zoom; }
	wxPoint WorldToScreen(glm::vec2 world) const;
	glm::vec2 ScreenToWorld(wxPoint screen) const;
	glm::vec2 MouseToWorld(wxMouseEvent &e) const;
	const glm::mat4 &GetMatrix() const { return m_view; }
private:
	void SetupMatrix();
	glm::mat4 m_view;
	float m_zoom  = 25.0f;
	glm::vec2 m_pan = { 0.0f, 0.0f };
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
	glm::vec2 m_lastmousepos;
	wxWindow *m_parent;
};


#endif
