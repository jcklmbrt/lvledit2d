
#ifndef _VIEWMATRIX_HPP
#define _VIEWMATRIX_HPP

#include <wx/geometry.h>
#include "src/geometry.hpp"

#define MAX_PAN_X  10000.0
#define MIN_PAN_X -10000.0
#define MAX_PAN_Y  10000.0
#define MIN_PAN_Y -10000.0
#define MAX_ZOOM 10.0
#define MIN_ZOOM 0.01

class ViewMatrix
{
public:
	ViewMatrix();
	void Pan(Point2D point);
	void Zoom(Point2D point, float factor);
	float GetZoom() const { return m_zoom; }
	wxPoint WorldToScreen(Point2D world) const;
	Point2D ScreenToWorld(wxPoint screen) const;
	Point2D MouseToWorld(wxMouseEvent &e) const;
	Matrix4 GetMatrix() const { return m_view; }
private:
	void SetupMatrix();
	Matrix4 m_view;
	float m_zoom  = 1.0;
	Point2D m_pan = { 0.0, 0.0 };
};


class ViewMatrixCtrl : public wxEvtHandler,
                       public ViewMatrix
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
	Point2D m_lastmousepos;
	wxWindow *m_parent;
};


#endif
