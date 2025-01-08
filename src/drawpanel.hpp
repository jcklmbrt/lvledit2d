
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <vector>
#include <wx/dcclient.h>
#include <wx/geometry.h>
#include <wx/wx.h>

#include "src/toolbar.hpp"
#include "src/convexpolygon.hpp"
#include "src/viewmatrix.hpp"

class IBaseEdit;

class DrawPanel : public wxPanel,
                  public ViewMatrix,
                  public std::vector<ConvexPolygon>
{
public:
	DrawPanel(wxWindow *parent);
	~DrawPanel();
	/* Editor helpers */
	int  GetGridSpacing() { return m_gridspacing; }
	bool IsSnapToGrid() { return m_snaptogrid; }
	wxPoint2DDouble GetMousePos() { return m_mousepos; }
	void FinishEdit();
	void OnToolSelect(ToolBar::ID id);
	ConvexPolygon *SelectPoly(wxPoint2DDouble wpos);
	ConvexPolygon *ClosestPoly(wxPoint2DDouble wpos, double threshold);
	static void DrawPoint(wxPaintDC &dc, wxPoint point, const wxColor *color);
private:
	/* Drawing */
	void OnPaint(wxPaintEvent &e);
	void DrawGrid(wxPaintDC &dc);
	/* Mouse events */
	void OnMouseLeftDown(wxMouseEvent &e);
	void OnMouseLeftUp(wxMouseEvent &e);
	void OnMouseMotion(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);
	/* Zoom/Pan ctrl */
	void OnMouseMiddleDown(wxMouseEvent &e);
	void OnMouseMiddleUp(wxMouseEvent &e);
	void OnMouseWheel(wxMouseEvent &e);

	/* cache mouse position for use in draw routines */
	wxPoint2DDouble m_mousepos;

	/* Zoom/Pan ctrl */
	bool m_inpan;
	wxPoint2DDouble m_lastmousepos;

	/* editing context */
	IBaseEdit *m_edit = nullptr;

	int m_gridspacing = 50;
	bool m_snaptogrid  = true;
};

#endif
