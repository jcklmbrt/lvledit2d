#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/geometry.h>
#include <wx/wx.h>
#include "src/plane2d.hpp"
#include "src/toolbar.hpp"
#include "src/convexpolygon.hpp"

class DrawPanel;
class EditorContext;


enum class EditActionType_t
{
	/* User actions */
	LINE,
	RECT,
	MOVE
};


class EditAction_Base
{
public:
	EditActionType_t type;
	size_t poly;
};


class EditAction_Rect : EditAction_Base
{
public:
	EditAction_Rect() { type = EditActionType_t::RECT; }
	wxRect2DDouble rect;
};


class EditAction_Line : EditAction_Base
{
public:
	EditAction_Line() { type = EditActionType_t::LINE; }
	wxRect2DDouble aabb;
	wxPoint2DDouble start;
	wxPoint2DDouble end;
	Plane2D plane;
};


class EditAction_Move : EditAction_Base
{
public:
	EditAction_Move() { type = EditActionType_t::MOVE; }
	wxPoint2DDouble delta;
};


union EditAction
{
	EditAction(EditAction_Line &line) : line(line) {}
	EditAction(EditAction_Rect &rect) : rect(rect) {}
	EditAction(EditAction_Move &move) : move(move) {}
	EditAction_Base base;
	EditAction_Rect rect;
	EditAction_Line line;
	EditAction_Move move;
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
	void ApplyAction(EditAction action);
	void Undo();
	void Redo();
	ConvexPolygon *SelectPoly(wxPoint2DDouble wpos);
	void ResetPoly(size_t i);
	void OnToolSelect(ToolBar::ID id);
	void OnPaint(wxPaintEvent &e);
	bool IsSnapToGrid() { return m_snaptogrid; }
	std::vector<ConvexPolygon> &GetPolys() { return m_polys; }
	std::vector<EditAction> &GetHistory() { return m_actions; }
	ConvexPolygon *GetSelectedPoly() { return m_selected; };
	void SetSelectedPoly(ConvexPolygon *poly) { m_selected = poly; }
	EditAction &LastAction() { wxASSERT(m_history); return m_actions[m_history - 1]; };
	size_t HistorySize() { return m_history; }
private:
	std::vector<ConvexPolygon> m_polys;
	std::vector<EditAction> m_actions;
	size_t m_history = 0; /* replacement for m_actions.size() */
	ConvexPolygon *m_selected = nullptr;
	IBaseEdit *m_state;
	DrawPanel *m_parent;
	bool m_snaptogrid = true;
};

#endif
