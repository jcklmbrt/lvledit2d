#ifndef _EDITORCONTEXT_HPP
#define _EDITORCONTEXT_HPP

#include <vector>
#include <wx/filename.h>
#include "glm/fwd.hpp"
#include "src/gl/texture.hpp"
#include "src/toolbar.hpp"
#include "src/geometry.hpp"
#include "src/edit/editorlayer.hpp"
#include "src/edit/editaction.hpp"

class GLCanvas;
class EditorContext;
class ViewMatrixBase;


class IBaseEdit : public wxEvtHandler
{
public:
	IBaseEdit(GLCanvas *canvas);
	virtual void DrawPolygon(const ConvexPolygon *p);
	virtual void OnDraw();
	virtual ~IBaseEdit();
protected:
	GLCanvas *m_canvas;
	EditorContext &m_edit;
	const ViewMatrixBase &m_view;
};

class EditorContext : public wxEvtHandler
{
public:
	EditorContext(GLCanvas *parent);
	~EditorContext();
	ConvexPolygon *ApplyAction(const ActData &action);
	ConvexPolygon *UndoAction(const ActData &action);
	void AddAction(const ActRect &rect);
	void AddAction(const ActLine &line);
	void AddAction(const ActMove &move);
	void AddAction(const ActScale &scale);
	void AddAction(const ActTexture &texture);
	void AddDelete();
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
	std::vector<GLTexture> m_textures;

	ActList m_actions;
	size_t m_selected = -1;
	size_t m_curlayer = -1;

	// file data
	wxString m_name;
	wxString m_path;
	bool m_hasfile;

	IBaseEdit *m_state;
	GLCanvas *m_canvas;
public:
	inline std::vector<EditorLayer> &GetLayers() { return m_layers; }
	inline const std::vector<EditorLayer> &GetLayers() const { return m_layers; }

	inline std::vector<GLTexture> &GetTextures() { return m_textures; }
	inline const std::vector<GLTexture> &GetTextures() const { return m_textures; }

	inline ActList &GetActList() { return m_actions; }

	inline bool HasFile() { return m_hasfile; }
	inline wxString &GetName() { return m_name; }

	inline size_t GetSelectedLayerIndex() { return m_curlayer; }
	inline void SetSelectedLayerIndex(size_t i) { m_curlayer = i; m_selected = -1; }

	inline EditorLayer *GetSelectedLayer()
	{
		size_t i = m_curlayer;
		if(i >= 0 && i < m_layers.size()) {
			return &m_layers[i];
		} else {
			return nullptr;
		}
	}

	inline ConvexPolygon *GetSelectedPoly()
	{
		size_t i = m_selected;
		EditorLayer *layer = GetSelectedLayer();
		if(layer == nullptr) {
			return nullptr;
		}

		std::vector<ConvexPolygon> &polys = layer->GetPolys();

		if(i < polys.size()) {
			return &polys[i];
		} else {
			return nullptr;
		}
	}

	inline void SetSelectedPoly(ConvexPolygon *p)
	{
		EditorLayer *layer = GetSelectedLayer();
		wxASSERT(layer);
		std::vector<ConvexPolygon> &polys = layer->GetPolys();

		size_t i = p - polys.data();
		if(i < polys.size()) {
			m_selected = i;
		}
	}

	inline void SetSelectedLayer(EditorLayer *layer)
	{
		size_t i = layer - m_layers.data();
		if(i < m_layers.size()) {
			m_selected = -1;
			m_curlayer = i;
		}
	}
};
#endif
