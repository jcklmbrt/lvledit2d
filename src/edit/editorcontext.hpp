#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "src/gl/texture.hpp"
#include "src/toolbar.hpp"
#include "src/geometry.hpp"

struct GLCanvas;
class EditorContext;
class ViewMatrixBase;


enum class EditActionType_t
{
	/* User actions */
	LINE,
	RECT,
	MOVE,
	TEXTURE
};


struct EditAction_Base
{
	EditActionType_t type;
	size_t poly;
};


struct EditAction_Rect : EditAction_Base
{
	EditAction_Rect() { type = EditActionType_t::RECT; }
	Rect2D rect;
};


struct EditAction_Line : EditAction_Base
{
	EditAction_Line() { type = EditActionType_t::LINE; }
	Rect2D aabb;
	Point2D start;
	Point2D end;
	Plane2D plane;
};


struct EditAction_Move : EditAction_Base
{
	EditAction_Move() { type = EditActionType_t::MOVE; }
	Point2D delta;
};


struct EditAction_Texture : EditAction_Base
{
	EditAction_Texture() { type = EditActionType_t::TEXTURE; }
	size_t index;
	int scale;
};


union EditAction
{
	EditAction() {};
	EditAction(EditAction_Line &line) : line(line) {}
	EditAction(EditAction_Rect &rect) : rect(rect) {}
	EditAction(EditAction_Move &move) : move(move) {}
	EditAction(EditAction_Texture &texture) : texture(texture) {}
	EditAction_Base base;
	EditAction_Rect rect;
	EditAction_Line line;
	EditAction_Move move;
	EditAction_Texture texture;
};


class IBaseEdit : public wxEvtHandler
{
public:
	IBaseEdit(GLCanvas *canvas);
	virtual void DrawPolygon(const ConvexPolygon *p);
	virtual void OnDraw();
	virtual ~IBaseEdit();
protected:
	GLCanvas *canvas;
	EditorContext *context;
	const ViewMatrixBase &view;
};


class EditorContext : public wxEvtHandler
{
public:
	EditorContext(GLCanvas *parent);
	~EditorContext();
	ConvexPolygon *ApplyAction(const EditAction &action);
	void AppendAction(EditAction action);
	void AddTexture(const wxFileName &filename);
	void Undo();
	void Redo();
	bool Save();
	bool Save(const wxFileName &path);
	bool Load(const wxFileName &path);
	ConvexPolygon *FindPoly(Point2D wpos);
	void ResetPoly(size_t i);
	void OnToolSelect(ToolBar::ID id);
	void OnDraw();
	ConvexPolygon *GetSelectedPoly();
	void SetSelectedPoly(ConvexPolygon *P);

	std::vector<ConvexPolygon> polys;
	std::vector<EditAction> actions;
	std::vector<GLTexture> textures;
	size_t history = 0; /* replacement for actions.size() */
	size_t selected = -1;
	wxString name;
	wxString path;
	bool has_file;
	IBaseEdit *state;
	GLCanvas *canvas;
	/* additional options that just default to true.
	   will add checkboxes later */
	bool snaptogrid = true;
};

inline ConvexPolygon *EditorContext::GetSelectedPoly()
{
	size_t i = selected;
	if(i >= 0 && i < polys.size()) {
		return &polys[i];
	} else {
		return nullptr;
	}
}

inline void EditorContext::SetSelectedPoly(ConvexPolygon *p)
{
	size_t i = p - polys.data();
	if(i >= 0 && i < polys.size()) {
		selected = i;
	}
}

#endif
