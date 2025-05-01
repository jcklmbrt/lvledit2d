#include <cstdio>
#include <numeric>
#include <wx/debug.h>
#include <wx/listbase.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>

#include "glm/fwd.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/geometry.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/textureedit.hpp"
#include "src/edit/entityedit.hpp"
#include "src/gl/texture.hpp"
#include "src/gl/glcontext.hpp"
#include "src/historylist.hpp"
#include "src/texturepanel.hpp"
#include "src/layerpanel.hpp"


bool EditorContext::Load(const wxFileName &filename)
{
	wxASSERT(filename.FileExists());

	m_name = filename.GetName();
	m_path = filename.GetFullPath();
	m_hasfile = true;

	L2dFile file;

	if(!file.LoadFromFile(m_path.c_str())) {
		return false;
	}

	if(!file.WriteToContext(this)) {
		return false;
	}

	ResetPolys();

	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(m_textures.size());
	tlist->Refresh();

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.TotalActions());
	hlist->Refresh();

	m_canvas->Refresh(true);

	return true;
}


bool EditorContext::Save()
{
	if(!m_hasfile) {
		return false;
	}

	L2dFile l2file;
	l2file.LoadFromContext(this);
	l2file.WriteToFile(m_path.c_str());

	return true;
}


bool EditorContext::Save(const wxFileName &filename)
{
	wxASSERT(filename.GetExt() == "l2d");

	m_name = filename.GetName();
	m_path = filename.GetFullPath();
	m_hasfile = true;

	return Save();
}


EditorContext::EditorContext(GLCanvas *canvas)
	: m_canvas(canvas),
	  m_name("untitled"),
	  m_hasfile(false),
	  m_state(nullptr)
{
	// default layer
	m_layers.emplace_back(*wxRED);
	m_selectedlayer = m_layers.size() - 1;

	LayerList *llist = LayerList::GetInstance();
	llist->SetItemCount(m_layers.size());
	llist->Refresh(true);
}


EditorContext::~EditorContext()
{
	if(m_state != nullptr) {
		m_canvas->RemoveEventHandler(m_state);
		delete m_state;
	}

	if(m_hasfile) {
		Save();
	}
}


void EditorContext::OnDraw()
{
	if(m_state != nullptr) {
		for(EditorLayer &layer : m_layers) {
			for(size_t &i : layer.GetPolys()) {
				m_state->DrawPolygon(&m_polys[i]);
			}
		}

		if(m_selectedpoly != -1) {
			const ConvexPolygon &poly = m_polys[m_selectedpoly];
			m_state->DrawPolygon(&poly);
		}

		m_state->OnDraw();
	}
}


void EditorContext::OnToolSelect(ToolBar::ID id)
{
	if(m_state != nullptr) {
		if(m_state != nullptr) {
			m_canvas->RemoveEventHandler(m_state);
			delete m_state;
		}
		m_state = nullptr;
		m_canvas->Refresh(false);
	}

	if(m_state == nullptr) {
		switch(id) {
		case ToolBar::ID::SELECT:
			m_state = new SelectionEdit(m_canvas);
			break;
		case ToolBar::ID::QUAD:
			m_state = new RectangleEdit(m_canvas);
			break;
		case ToolBar::ID::LINE:
			m_state = new LineEdit(m_canvas);
			break;
		case ToolBar::ID::TEXTURE:
			m_state = new TextureEdit(m_canvas);
			break;
		case ToolBar::ID::ENTITY:
			m_state = new EntityEdit(m_canvas);
			break;
		default:
			break;
		}
		if(m_state != nullptr) {
			m_canvas->PushEventHandler(m_state);
		}
	}
}


void EditorContext::ResetPolys()
{
	for(EditorLayer &layer : m_layers) {
		layer.GetPolys().clear();
	}

	m_polys.clear();

	size_t hindex = m_actions.HistoryIndex();
	for(size_t i = 0; i < hindex; i++) {
		ActData act;
		m_actions.GetAction(i, act);
		ApplyAction(act);
	}
}


void EditorContext::Undo()
{
	ActData act;
	if(!m_actions.Undo(act)) {
		return;
	}

	if(act.layer < 0 || act.layer >= m_layers.size()) {
		return;
	}

	UndoAction(act);

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.TotalActions());
	hlist->Refresh();
	Save();
}

void EditorContext::ApplyAction(const ActData &act)
{
	ConvexPolygon *poly = nullptr;

	if(act.type == ActType::LAYER) {
		wxColour col;
		col.SetRGB(act.layer);
		m_layers.push_back(col);
		return;
	}

	if(act.layer < 0 || act.layer >= m_layers.size()) {
		return;
	}

	EditorLayer &layer = m_layers[act.layer];

	switch(act.type) {
	case ActType::LINE:
		poly = &m_polys[act.poly];
		poly->Slice(act.line);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case ActType::MOVE:
		poly = &m_polys[act.poly];
		poly->Offset(act.move);
		break;
	case ActType::SCALE:
		poly = &m_polys[act.poly];
		poly->Scale(act.scale.origin, act.scale.numer, act.scale.denom);
		break;
	case ActType::RECT:
		poly = &m_polys.emplace_back(act.rect);
		layer.AddPoly(act.poly);
		break;
	case ActType::DEL:
		poly = nullptr;
		if(act.poly == -1) {
			m_layers.erase(m_layers.begin() + act.layer);
		} else {
			layer.RemovePoly(act.poly);
		}
		return;
	case ActType::TEXTURE:
		poly = &m_polys[act.poly];
		poly->SetTexture(act.texture.index, act.texture.scale);
		break;
	case ActType::LAYER:
		break;
	}

	wxASSERT(poly);
	wxASSERT_MSG(act.poly == poly - &m_polys.front(),
		"Polygon indices out of order.");

	m_selectedpoly = act.poly;
}

ConvexPolygon *EditorContext::FindPoly(glm::vec2 p)
{
	if(m_selectedlayer == -1) {
		return nullptr;
	}

	const EditorLayer &layer = m_layers[m_selectedlayer];
	for(size_t i : layer.GetPolys()) {
		if(m_polys[i].Contains(p)) {
			return &m_polys[i];
		}
	}

	return nullptr;
}


void EditorContext::DeleteLayer()
{
	if(!m_layers.empty() && m_selectedlayer != -1) {
		m_actions.AddDelete(-1, m_selectedlayer);
		ActData back;
		m_actions.GetBack(back);
		ApplyAction(back);
	}
}


void EditorContext::UndoAction(const ActData &act)
{
	ConvexPolygon *poly = nullptr;
	EditorLayer &layer = m_layers[act.layer];

	switch(act.type) {
	case ActType::TEXTURE:
	case ActType::LINE:
	case ActType::DEL:
		/* HACK: completely resets *everything* after each undo.
		   Instead it would be better to fix broken indices. */
		ResetPolys();
		break;
	case ActType::MOVE:
		poly = &m_polys[act.poly];
		poly->Offset(-act.move);
		break;
	case ActType::SCALE:
		poly = &m_polys[act.poly];
		poly->Scale(act.scale.origin, act.scale.denom, act.scale.numer);
		break;
	case ActType::RECT:
		m_selectedpoly = -1;
		layer.RemovePoly(act.poly);
		break;
	case ActType::LAYER:
		m_selectedlayer = -1;
		m_layers.erase(m_layers.begin() + act.layer);
		break;
	}
}


void EditorContext::AddTexture(const wxFileName &filename)
{
	GLTexture texture;
	texture.Load(filename);

	for(GLTexture &t : m_textures) {
		if(texture == t) {
			return;
		}
	}

	m_textures.push_back(texture);
	m_selectedtexture = m_textures.size() - 1;

	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(m_textures.size());
	tlist->Refresh();
}


void EditorContext::Redo()
{
	ActData act;
	if(m_actions.Redo(act)) {
		ApplyAction(act);
		HistoryList *hlist = HistoryList::GetInstance();
		hlist->SetItemCount(m_actions.TotalActions());
		hlist->Refresh();
	}
}


void EditorContext::AddAction(const ActRect &rect)
{
	m_actions.AddAction(rect, m_polys.size(), m_selectedlayer);

	ActData back;
	m_actions.GetBack(back);
	ApplyAction(back);

	m_selectedpoly = m_polys.size() - 1;

	Save();
	m_canvas->Refresh();
}

void EditorContext::AddAction(const ActLayer &layer)
{
	m_selectedpoly = -1;
	m_selectedlayer = m_layers.size();
	
	m_actions.AddAction(layer, m_selectedlayer);

	ActData back;
	m_actions.GetBack(back);
	ApplyAction(back);
	Save();

	LayerList *list = LayerList::GetInstance();
	list->SetItemCount(m_layers.size());

	m_canvas->Refresh();
}


void EditorContext::AddAction(const ActLine &line)
{
	if(m_selectedpoly == -1 || m_selectedlayer == -1) {
		return;
	}

	m_actions.AddAction(line, m_selectedpoly, m_selectedlayer);

	ActData back;
	m_actions.GetBack(back);

	ApplyAction(back);
	Save();

	m_canvas->Refresh();
}


void EditorContext::AddAction(const ActMove &move)
{
	if(move.x == 0 && move.y == 0) {
		return;
	}

	ActData back;
	m_actions.GetBack(back);

	if(back.type == ActType::MOVE && back.poly == m_selectedpoly) {
		/* we don't want to spam a move action for each pixel moved */
		UndoAction(back);

		back.move += move;

		/* remove if no-op */
		if(back.move.x == 0 && back.move.y == 0) {
			m_actions.PopBack();
			return;
		}

		m_actions.UpdateBack(back);
		Save();
	} else {
		m_actions.AddAction(move, m_selectedpoly, m_selectedlayer);
	}

	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}

void EditorContext::AddAction(const ActScale &scale)
{
	ActData back;
	bool add_action = true;

	size_t layer = m_selectedlayer;
	size_t poly = m_selectedpoly;

	if(!m_actions.IsEmpty() && m_actions.HistoryIndex()) {
		m_actions.GetBack(back);
		if(back.type == ActType::SCALE &&
		   back.poly == poly &&
		   back.layer == layer &&
		   back.scale.origin == scale.origin) {

			UndoAction(back);

			back.scale.denom *= scale.denom;
			back.scale.numer *= scale.numer;

			glm::i32vec2 g;
			g.x = std::gcd(back.scale.numer.x, back.scale.denom.x);
			g.y = std::gcd(back.scale.numer.y, back.scale.denom.y);
			back.scale.numer /= g;
			back.scale.denom /= g;

			/* remove if no-op */
			if(back.scale.numer == back.scale.denom) {
				m_actions.PopBack();
				Save();
				return;
			}

			m_actions.UpdateBack(back);
			add_action = false;
		}
	}

	if(add_action) {
		m_actions.AddAction(scale, poly, layer);
	}

	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}

void EditorContext::AddAction(const ActTexture &texture)
{
	if(m_selectedpoly == -1 || m_selectedlayer == -1) {
		return;
	}

	m_polys[m_selectedpoly].SetTexture(texture.index, texture.scale);

	ActData back;
	if(!m_actions.IsEmpty() && m_actions.HistoryIndex()) {
		m_actions.GetBack(back);
		/* don't bother saving repeat texture actions */
		if(back.type == ActType::TEXTURE) {
			if(back.poly == m_selectedpoly
				&& texture.index == back.texture.index
				&& texture.scale == back.texture.scale) {
				return;
			}
		}
	}

	m_actions.AddAction(texture, m_selectedpoly, m_selectedlayer);
	Save();
	m_canvas->Refresh();
}

void EditorContext::AddDelete()
{
	LayerList *llist = LayerList::GetInstance();
	llist->Refresh();

	m_actions.AddDelete(m_selectedpoly, m_selectedlayer);
	m_selectedpoly = -1;

	ActData back;
	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}
