#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "glm/fwd.hpp"
#include "src/gl/texture.hpp"
#include "src/toolbar.hpp"
#include "src/geometry.hpp"

struct GLCanvas;
class EditorContext;
class ViewMatrixBase;


enum class EditActionType
{
	/* User actions */
	LINE,
	RECT,
	MOVE,
	SCALE,
	DEL,
	TEXTURE
};


struct EditAction_Base
{
	EditActionType type;
	size_t poly;
};


struct EditAction_Rect : EditAction_Base
{
	EditAction_Rect() { type = EditActionType::RECT; }
	Rect2D rect;
};


struct EditAction_Line : EditAction_Base
{
	EditAction_Line() { type = EditActionType::LINE; }
	Plane2D plane;
};

struct EditAction_Move : EditAction_Base
{
	EditAction_Move() { type = EditActionType::MOVE; }
	glm::i32vec2 delta;
};

struct EditAction_Scale : EditAction_Base
{
	EditAction_Scale() { type = EditActionType::SCALE; }
	glm::i32vec2 origin;
	glm::i32vec2 numer;
	glm::i32vec2 denom;
};


struct EditAction_Texture : EditAction_Base
{
	EditAction_Texture() { type = EditActionType::TEXTURE; }
	int32_t index;
	int32_t scale;
};

struct EditAction_Delete : EditAction_Base
{
	EditAction_Delete() { type = EditActionType::DEL; }
};


union EditAction
{
	EditAction() {};
	EditAction(EditAction_Line &line) : line(line) {}
	EditAction(EditAction_Rect &rect) : rect(rect) {}
	EditAction(EditAction_Move &move) : move(move) {}
	EditAction(EditAction_Scale &scale) : scale(scale) {}
	EditAction(EditAction_Texture &texture) : texture(texture) {}
	EditAction(EditAction_Delete &del) : del(del) {}
	EditAction_Base base;
	EditAction_Rect rect;
	EditAction_Line line;
	EditAction_Move move;
	EditAction_Scale scale;
	EditAction_Delete del;
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
	ConvexPolygon *FindPoly(glm::vec2 wpos);
	void ResetPolys();
	void OnToolSelect(ToolBar::ID id);
	void OnDraw();
	ConvexPolygon *GetSelectedPoly();
	void SetSelectedPoly(ConvexPolygon *p);

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
