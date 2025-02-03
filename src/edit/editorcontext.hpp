#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "src/gl/texture.hpp"
#include "src/toolbar.hpp"
#include "src/geometry.hpp"
#include "src/gl/gltexturegeometry.hpp"

class GLCanvas;
class EditorContext;
class ViewMatrixBase;


enum EditActionType_t
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
	size_t Index;
	int Scale;
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
	GLCanvas *Canvas;
	EditorContext *Context;
	const ViewMatrixBase &View;
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
	ConvexPolygon *SelectPoly(Point2D wpos);
	void ResetPoly(size_t i);
	void OnToolSelect(ToolBar::ID id);
	void OnDraw();
	ConvexPolygon *GetSelectedPoly();
	void SetSelectedPoly(ConvexPolygon *P);

	std::vector<ConvexPolygon> Polygons;
	std::vector<EditAction> Actions;
	std::vector<GLTexture> Textures;
	size_t History = 0; /* replacement for m_actions.size() */
	size_t SelectedPolygon = -1;
	FILE *File = nullptr;
	wxString Name;
	IBaseEdit *State;
	GLCanvas *Canvas;
	/* additional options that just default to true.
	   will add checkboxes later */
	bool SnapToGrid = true;
};

inline ConvexPolygon *EditorContext::GetSelectedPoly()
{
	size_t i = SelectedPolygon;
	if(i >= 0 && i < Polygons.size()) {
		return &Polygons[i];
	} else {
		return nullptr;
	}
}

inline void EditorContext::SetSelectedPoly(ConvexPolygon *P)
{
	size_t Idx = P - Polygons.data();
	if(Idx > 0 && Idx < Polygons.size()) {
		SelectedPolygon = Idx;
	}
}

#endif
