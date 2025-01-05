
#ifndef _VIEWMATRIX_HPP
#define _VIEWMATRIX_HPP

#include <wx/affinematrix2d.h>
#include <wx/event.h>

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
	void SetupMatrix();
	void Pan(wxPoint2DDouble point);
	void Zoom(wxPoint2DDouble point, double factor);
	wxPoint WorldToScreen(wxPoint2DDouble world);
	wxPoint2DDouble ScreenToWorld(wxPoint screen);
	wxPoint2DDouble MouseToWorld(wxMouseEvent &e);
private:
	wxAffineMatrix2D m_matrix;
	double m_zoom  = 1.0;
	wxPoint2DDouble m_pan = { 0.0, 0.0 };
};

#endif
