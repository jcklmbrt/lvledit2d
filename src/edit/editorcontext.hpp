#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "src/toolbar.hpp"
#include "src/geometry.hpp"

class GLCanvas;
class EditorContext;
class ViewMatrix;


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
	Rect2D rect;
};


class EditAction_Line : EditAction_Base
{
public:
	EditAction_Line() { type = EditActionType_t::LINE; }
	Rect2D aabb;
	Point2D start;
	Point2D end;
	Plane2D plane;
};


class EditAction_Move : EditAction_Base
{
public:
	EditAction_Move() { type = EditActionType_t::MOVE; }
	Point2D delta;
};


union EditAction
{
	EditAction() {}
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
	IBaseEdit(GLCanvas *canvas);
	virtual void DrawPolygon(const ConvexPolygon *p);
	virtual void OnDraw();
	virtual ~IBaseEdit();
protected:
	GLCanvas *m_canvas;
	EditorContext *m_context;
	const ViewMatrix &m_view;
};


class EditorContext : public wxEvtHandler
{
public:
	EditorContext(GLCanvas *parent);
	~EditorContext();
	ConvexPolygon *ApplyAction(const EditAction &action);
	void AppendAction(EditAction action);
	void Undo();
	void Redo();
	bool Save();
	bool Save(const wxFileName &path);
	bool Load(const wxFileName &path);
	ConvexPolygon *SelectPoly(Point2D wpos);
	void ResetPoly(size_t i);
	void OnToolSelect(ToolBar::ID id);
	void OnDraw();
	bool IsSnapToGrid() { return m_snaptogrid; }
	std::vector<ConvexPolygon> &GetPolys() { return m_polys; }
	std::vector<EditAction> &GetHistory() { return m_actions; }
	ConvexPolygon *GetSelectedPoly() { return m_selected; };
	void SetSelectedPoly(ConvexPolygon *poly) { m_selected = poly; }
	EditAction &LastAction() { wxASSERT(m_history); return m_actions[m_history - 1]; };
	size_t HistorySize() { return m_history; }
	wxString GetName() { return m_name; }
	bool HasFile() { return m_file != nullptr; }
private:
	std::vector<ConvexPolygon> m_polys;
	std::vector<EditAction> m_actions;
	size_t m_history = 0; /* replacement for m_actions.size() */
	ConvexPolygon *m_selected = nullptr;
	FILE *m_file = nullptr;
	wxString m_name;
	IBaseEdit *m_state;
	GLCanvas *m_canvas;
	/* additional options that just default to true.
	   will add checkboxes later */
	bool m_snaptogrid = true;
};

#endif
