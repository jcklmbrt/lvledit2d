#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "glm/fwd.hpp"
#include "src/gl/texture.hpp"
#include "src/toolbar.hpp"
#include "src/geometry.hpp"
#include "src/edit/editorlayer.hpp"

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
	size_t layer;
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

struct EditAction_Index
{
	EditActionType type;
	size_t index;
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
	GLCanvas *m_canvas;
	EditorContext *m_context;
	const ViewMatrixBase &m_view;
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
private:
	std::vector<EditorLayer> m_layers;
	std::vector<EditAction> m_actions;
	std::vector<GLTexture> m_textures;

	/* actions[0..history] --> history */
	/* actions[history..size] -> future  */
	size_t m_history = 0;

	size_t m_selected = -1;
	size_t m_curlayer = -1;

	// file data
	wxString m_name;
	wxString m_path;
	bool m_hasfile;

	IBaseEdit *m_state;
	GLCanvas *m_canvas;
public:
	size_t NumLayers() { return m_layers.size(); }
	EditorLayer &GetLayer(size_t i) { return m_layers.at(i); }
	std::vector<EditorLayer> &GetLayers() { return m_layers; }

	size_t NumTextures() { return m_textures.size(); }
	GLTexture &GetTexture(size_t i) { return m_textures.at(i); }

	size_t NumActions() { return m_actions.size(); }
	EditAction &GetAction(size_t i) { return m_actions.at(i); }
	size_t HistoryIndex() { return m_history; }

	bool HasFile() { return m_hasfile; }
	wxString &GetName() { return m_name; }

	EditorLayer *GetSelectedLayer()
	{
		size_t i = m_curlayer;
		if(i >= 0 && i < m_layers.size()) {
			return &m_layers[i];
		} else {
			return nullptr;
		}
	}

	ConvexPolygon *GetSelectedPoly()
	{
		size_t i = m_selected;
		EditorLayer *layer = GetSelectedLayer();
		if(layer == nullptr) {
			return nullptr;
		}

		std::vector<ConvexPolygon> &polys = layer->GetPolys();

		if(i >= 0 && i < polys.size()) {
			return &polys[i];
		} else {
			return nullptr;
		}
	}

	void SetSelectedPoly(ConvexPolygon *p)
	{
		EditorLayer *layer = GetSelectedLayer();
		wxASSERT(layer);
		std::vector<ConvexPolygon> &polys = layer->GetPolys();

		size_t i = p - polys.data();
		if(i >= 0 && i < polys.size()) {
			m_selected = i;
		}
	}

	void SetSelectedLayer(EditorLayer *layer)
	{
		size_t i = layer - m_layers.data();
		if(i >= 0 && i < m_layers.size()) {
			m_selected = -1;
			m_curlayer = i;
		}
	}
};
#endif
