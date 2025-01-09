#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/wx.h>
#include "src/toolbar.hpp"
#include "src/convexpolygon.hpp"

class DrawPanel;
class EditorContext;

class IBaseEdit : public wxEvtHandler
{
public:
	IBaseEdit(DrawPanel *panel);
	virtual void DrawPolygon(wxPaintDC &dc, const ConvexPolygon *p);
	virtual ~IBaseEdit();
	DrawPanel *GetPanel();
protected:
	DrawPanel *m_panel;
	EditorContext *m_context;
	ConvexPolygon *m_poly;
};


class EditorContext : public wxEvtHandler
{
public:
	EditorContext(DrawPanel *parent);
	ConvexPolygon *SelectPoly(wxPoint2DDouble wpos);
	ConvexPolygon *ClosestPoly(wxPoint2DDouble wpos, double threshold);
	void FinishEdit();
	void OnToolSelect(ToolBar::ID id);
	bool IsSnapToGrid() { return m_snaptogrid; }
	std::vector<ConvexPolygon> &GetPolys() { return m_polys; }
	void OnPaint(wxPaintEvent &e);
private:
	std::vector<ConvexPolygon> m_polys;
	ConvexPolygon *m_selected = nullptr;
	IBaseEdit *m_state;
	DrawPanel *m_parent;
	bool m_snaptogrid;
};

#endif