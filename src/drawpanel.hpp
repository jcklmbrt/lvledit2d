
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <wx/geometry.h>
#include <wx/vector.h>
#include <wx/wx.h>

struct ConnectLine
{
	int rect[2]; /* index into m_rects */
	int outcode[2];

	inline bool GetPoints(const wxVector<wxRect2DDouble> &rects,
	              wxPoint2DDouble p[2]) const 
	{
		for(int i = 0; i < 2; i++) {
			wxRect2DDouble r = rects[rect[i]];
			switch(outcode[i]) {
			case wxOutLeft  + wxOutTop:    p[i] = r.GetLeftTop();     break;
			case wxOutRight + wxOutTop:    p[i] = r.GetRightTop();    break;
			case wxOutLeft  + wxOutBottom: p[i] = r.GetLeftBottom();  break;
			case wxOutRight + wxOutBottom: p[i] = r.GetRightBottom(); break;
			default: return false;
			}
		}
		return true;
	}
};

class DrawPanel : public wxPanel
{
	static constexpr double MAX_PAN_X = 10000.0;
	static constexpr double MIN_PAN_X = -10000.0;
	static constexpr double MAX_PAN_Y = 10000.0;
	static constexpr double MIN_PAN_Y = -10000.0;
	static constexpr double MAX_ZOOM = 10.0;
	static constexpr double MIN_ZOOM = 0.01;
public:
	DrawPanel(wxWindow *parent);
	inline void FinishEdit() { m_inedit = 0; }
private:
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	void SetupView();
	void Pan(wxPoint2DDouble point);
	void Zoom(wxPoint2DDouble point, double factor);
	void WorldToScreen(wxPoint2DDouble world, wxPoint &screen);
	void ScreenToWorld(wxPoint screen, wxPoint2DDouble &world);

	bool FindClosestRectCorner(wxPoint2DDouble &pt, int &corner, int &rect);
	void DrawRect(wxPaintDC &dc, wxRect2DDouble rect, wxColour color, bool tmp);

	wxAffineMatrix2D m_view;
	wxPoint2DDouble  m_pan  = { 0.0, 0.0 };
	double           m_zoom = 1.0;

	wxPoint2DDouble  m_mousepos;

	wxVector<wxRect2DDouble> m_rects;
	wxVector<ConnectLine>    m_lines;

	/* editing context */
	int             m_inedit = 0;

	wxRect2DDouble  m_tmprect;
	wxPoint2DDouble m_editstart;
	ConnectLine     m_tmpline;

	int  m_gridspacing = 100;
	bool m_snaptogrid  = false;

	wxDECLARE_EVENT_TABLE();
};

#endif
