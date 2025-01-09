
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <vector>
#include <wx/dcclient.h>
#include <wx/geometry.h>
#include <wx/wx.h>

#include "src/toolbar.hpp"
#include "src/convexpolygon.hpp"
#include "src/viewmatrix.hpp"
#include "src/edit/editorcontext.hpp"

class DrawPanel;

class BackgroundGrid : public wxEvtHandler
{
public:
	constexpr static int SPACING = 50;
	BackgroundGrid(DrawPanel *parent);
	static void Snap(wxPoint2DDouble &pt);
private:
	void OnPaint(wxPaintEvent &e);
private:
	DrawPanel *m_parent;
};


class DrawPanel : public wxPanel
{
public:
	DrawPanel(wxWindow *parent);
	~DrawPanel();
	/* Editor helpers */
	int  GetGridSpacing() { return m_grid.SPACING; }
	bool IsSnapToGrid() { return m_editor.IsSnapToGrid(); }
	wxPoint2DDouble GetMousePos() { return m_mousepos; }
	EditorContext &GetEditor() { return m_editor; };
	static void DrawPoint(wxPaintDC &dc, wxPoint point, const wxColor *color);
private:
	/* Drawing */
	void OnPaint(wxPaintEvent &e);
	/* Mouse events */
	void OnMouse(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	/* cache mouse position for use in draw routines */
	wxPoint2DDouble m_mousepos;

	BackgroundGrid  m_grid;
	ViewMatrixCtrl  m_view;
	EditorContext   m_editor;
public:
	inline wxPoint WorldToScreen(wxPoint2DDouble world) { return m_view.WorldToScreen(world); }
	inline wxPoint2DDouble ScreenToWorld(wxPoint screen) {return m_view.ScreenToWorld(screen);}
	inline wxPoint2DDouble MouseToWorld(wxMouseEvent &e) { return m_view.MouseToWorld(e); }
};

#endif
