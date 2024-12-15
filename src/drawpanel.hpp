
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <vector>
#include <wx/geometry.h>
#include <wx/wx.h>

#include "src/convexpolygon.hpp"
#include "src/viewmatrix.hpp"

class IBaseEdit;

class DrawPanel : public wxPanel,
                  public ViewMatrix,
                  public std::vector<ConvexPolygon>
{
public:
	DrawPanel(wxWindow *parent);
	void FinishEdit();
	int  GetGridSpacing() { return m_gridspacing; }
	bool IsSnapToGrid()   { return m_snaptogrid; }
private:
	void OnPaint(wxPaintEvent &e);
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
	bool            m_inpan;
	wxPoint2DDouble m_lastmousepos;

	/* editing context */
	IBaseEdit *m_edit = nullptr;

	int  m_gridspacing = 50;
	bool m_snaptogrid  = true;
};

#endif
