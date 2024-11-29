
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <wx/geometry.h>
#include <wx/wx.h>

class DrawPanel : public wxPanel
{
	static constexpr wxDouble MAX_PAN_X = 10000.0;
	static constexpr wxDouble MIN_PAN_X = -10000.0;
	static constexpr wxDouble MAX_PAN_Y = 10000.0;
	static constexpr wxDouble MIN_PAN_Y = -10000.0;
	static constexpr wxDouble MAX_ZOOM = 10.0;
	static constexpr wxDouble MIN_ZOOM = 0.01;
public:
	DrawPanel(wxWindow *parent);
private:
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	void SetupView();
	void Pan(wxPoint2DDouble point);
	void Zoom(wxPoint2DDouble point, wxDouble factor);
	void WorldToScreen(wxPoint2DDouble world, wxPoint &screen);
	void ScreenToWorld(wxPoint screen, wxPoint2DDouble &world);

	wxAffineMatrix2D m_view;
	wxPoint2DDouble  m_pan  = { 0.0, 0.0 };
	wxDouble         m_zoom = 1.0;

	wxVector<wxRect> m_rects;

	wxDECLARE_EVENT_TABLE();
};

#endif
