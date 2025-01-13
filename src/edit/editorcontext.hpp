#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/wx.h>
#include "src/toolbar.hpp"
#include "src/convexpolygon.hpp"

class DrawPanel;
class EditorContext;


enum class EditActionType_t
{
	LINE,
	RECT,
	MOVE
};


struct EditAction_Line
{
	EditActionType_t type;
	size_t plane;
};


union EditAction
{
	EditActionType_t type;
	EditAction_Line  line;
};


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
};


class EditorContext : public wxEvtHandler
{
public:
	EditorContext(DrawPanel *parent);
	~EditorContext();
	ConvexPolygon *SelectPoly(wxPoint2DDouble wpos);
	ConvexPolygon *ClosestPoly(wxPoint2DDouble wpos, double threshold);
	void OnToolSelect(ToolBar::ID id);
	bool IsSnapToGrid() { return m_snaptogrid; }
	std::vector<ConvexPolygon> &GetPolys() { return m_polys; }
	void OnPaint(wxPaintEvent &e);
	ConvexPolygon *GetSelectedPoly() { return m_selected; };
	void SetSelectedPoly(ConvexPolygon *poly) { m_selected = poly; }
private:
	std::vector<wxRect2DDouble> m_rects;
	std::vector<Plane2D>        m_planes;
	std::vector<ConvexPolygon>  m_polys;
	ConvexPolygon *m_selected = nullptr;
	IBaseEdit *m_state;
	DrawPanel *m_parent;
	bool m_snaptogrid = true;
};

#endif