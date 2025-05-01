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
class IBaseEdit;

class EditorContext : public wxEvtHandler
{
public:
	EditorContext(GLCanvas *parent);
	~EditorContext();
	void ApplyAction(const ActData &action);
	void UndoAction(const ActData &action);
	void AddAction(const ActLayer &layer);
	void AddAction(const ActRect &rect);
	void AddAction(const ActLine &line);
	void AddAction(const ActMove &move);
	void AddAction(const ActScale &scale);
	void AddAction(const ActTexture &texture);
	void AddDelete();
	void AddTexture(const wxFileName &filename);
	void DeleteLayer();
	void Undo();
	void Redo();
	bool Save();
	bool Save(const wxFileName &path);
	bool Load(const wxFileName &path);
	void ResetPolys();
	ConvexPolygon *FindPoly(glm::vec2 mpos);
	void OnToolSelect(ToolBar::ID id);
	void OnDraw();
private:
	std::vector<EditorLayer> m_layers;
	std::vector<ConvexPolygon> m_polys;
	std::vector<GLTexture> m_textures;

	size_t m_selectedpoly = -1;
	size_t m_selectedlayer = -1;
	size_t m_selectedtexture = -1;
	ActList m_actions;

	wxString m_name;

	// file data
	wxString m_path;
	bool m_hasfile;

	IBaseEdit *m_state;
	GLCanvas *m_canvas;
public:
	inline std::vector<EditorLayer> &GetLayers() { return m_layers; }
	inline const std::vector<EditorLayer> &GetLayers() const { return m_layers; }
	inline std::vector<ConvexPolygon> &GetPolys() { return m_polys; }

	inline std::vector<GLTexture> &GetTextures() { return m_textures; }
	inline const std::vector<GLTexture> &GetTextures() const { return m_textures; }

	inline ActList &GetActList() { return m_actions; }
	inline const ActList &GetActList() const { return m_actions; }

	inline bool HasFile() { return m_hasfile; }
	inline wxString &GetName() { return m_name; }

	inline ConvexPolygon *GetSelectedPoly()
	{
		if(m_selectedpoly == -1) {
			return nullptr;
		} else {
			return &m_polys[m_selectedpoly];
		}
	}

	inline size_t GetSelectedTextureIndex()
	{
		return m_selectedtexture;
	}

	inline void SetSelectedTextureIndex(size_t index)
	{
		m_selectedtexture = index;
	}

	inline void SetSelectedPoly(ConvexPolygon *poly)
	{
		m_selectedpoly = poly - &m_polys.front();
	}

	inline void SetSelectedLayer(EditorLayer *layer)
	{
		m_selectedlayer = layer - &m_layers.front();
	}

	inline EditorLayer *GetSelectedLayer()
	{
		if(m_selectedlayer == -1 || m_layers.empty()) {
			return nullptr;
		} else {
			return &m_layers[m_selectedlayer];
		}
	}

	inline void SetSelectedLayerIndex(size_t index)
	{
		m_selectedlayer = index;
	}
};
#endif
