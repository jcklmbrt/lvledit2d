#include <array>
#include <cfloat>
#include <numeric>
#include <cstdio>
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

struct L2dHeader 
{
	uint8_t magic[2];
	uint32_t actions_offset;
	uint32_t actions_size;
	uint32_t actions_index;
	uint32_t texinfo_offset;
	uint32_t texinfo_size;
	uint32_t texdata_offset;
	uint32_t texdata_size;
};

bool EditorContext::Load(const wxFileName &filename)
{
	wxASSERT(filename.FileExists());

	m_name = filename.GetName();
	m_path = filename.GetFullPath();
	m_hasfile = true;
	FILE *file = fopen(m_path.c_str(), "rb");

	L2dHeader hdr;
	fseek(file, 0, SEEK_SET);
	fread(&hdr, sizeof(L2dHeader), 1, file);

	if(hdr.magic[0] != 'L' && hdr.magic[1] != '2') {
		fclose(file);
		file = nullptr;
		return false;
	}

	size_t num_actions = hdr.actions_size / sizeof(EditAction);
	m_actions.resize(num_actions);
	m_history = hdr.actions_index;

	fseek(file, hdr.actions_offset, SEEK_SET);
	fread(m_actions.data(), hdr.actions_size, 1, file);

	std::vector<TexInfo> texinfo;
	texinfo.resize(hdr.texinfo_size / sizeof(TexInfo));
	fseek(file, hdr.texinfo_offset, SEEK_SET);
	fread(texinfo.data(), hdr.texinfo_size, 1, file);

	std::vector<unsigned char> texdata;
	texdata.resize(hdr.texdata_size);
	fseek(file, hdr.texdata_offset, SEEK_SET);
	fread(texdata.data(), hdr.texdata_size, 1, file);

	for(TexInfo &info : texinfo) {
		size_t nbytes = info.width * info.height * info.pixelwidth;
		unsigned char *data = new unsigned char[nbytes];

		memcpy(data, texdata.data() + info.dataoffset, nbytes);

		GLTexture texture;
		texture.Load(info.width, info.height, info.pixelwidth, info.name, data);

		m_textures.push_back(texture);
	}

	ResetPolys();

	wxPoint2DDouble d;
	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(m_textures.size());
	tlist->Refresh();

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	m_canvas->Refresh(true);

	return true;
}


bool EditorContext::Save()
{
	if(!m_hasfile) {
		return false;
	}

	FILE *file = fopen(m_path.c_str(), "wb");

	std::vector<unsigned char> texdata;
	std::vector<TexInfo> texinfo;

	for(GLTexture &texture : m_textures) {
		TexInfo &info = texinfo.emplace_back();
		texture.AddToFile(info, texdata);
	}

	L2dHeader hdr;
	hdr.magic[0] = 'L';
	hdr.magic[1] = '2';
	hdr.actions_offset = sizeof(L2dHeader);
	hdr.actions_size = m_actions.size() * sizeof(EditAction);
	hdr.actions_index = m_history;
	hdr.texinfo_offset = hdr.actions_offset + hdr.actions_size;
	hdr.texinfo_size = texinfo.size() * sizeof(TexInfo);
	hdr.texdata_offset = hdr.texinfo_offset + hdr.texinfo_size;
	hdr.texdata_size = texdata.size();

	fseek(file, 0, SEEK_SET);
	fwrite(&hdr, sizeof(L2dHeader), 1, file);

	fseek(file, hdr.actions_offset, SEEK_SET);
	fwrite(m_actions.data(), hdr.actions_size, 1, file);

	fseek(file, hdr.texinfo_offset, SEEK_SET);
	fwrite(texinfo.data(), hdr.texinfo_size, 1, file);

	fseek(file, hdr.texdata_offset, SEEK_SET);
	fwrite(texdata.data(), hdr.texdata_size, 1, file);

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

void IBaseEdit::DrawPolygon(const ConvexPolygon *p)
{
	if(p == m_context->GetSelectedPoly()) {
		Rect2D aabb = p->aabb;
		m_canvas->OutlineRect(aabb, 1.0f, BLUE);

		/* inflate aabb to illustrate planes */
		aabb.mins -= 1;
		aabb.maxs += 1;

		std::array aabbpts = {
			glm::i32vec2 { aabb.mins.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.mins.y },
			glm::i32vec2 { aabb.maxs.x, aabb.maxs.y },
			glm::i32vec2 { aabb.mins.x, aabb.maxs.y }
		};

		for(const Plane2D &plane : p->planes) {
			glm::vec2 line[2]{};
			size_t n = 0;

			for(size_t i = 0; i < aabbpts.size(); i++) {
				glm::i64vec2 p1 = aabbpts[i];
				glm::i64vec2 p2 = aabbpts[(i + 1) % aabbpts.size()];
				glm::i64vec2 delta = p2 - p1;

				int64_t a = plane.a;
				int64_t b = plane.b;
				int64_t c = plane.c;

				int64_t denom = a * delta.x + b * delta.y;
				int64_t numer = -(a * p1.x + b * p1.y + c);

				if(denom != 0) {
					int64_t g = std::gcd(denom, numer);
					if(g != 0) {
						denom /= g;
						numer /= g;
					}

					float t = static_cast<float>(numer) / static_cast<float>(denom);
					if(t >= 0.0f && t <= 1.0f) {
						line[n++] = glm::vec2(p1) + t * glm::vec2(delta);
					}
				} else if(numer == 0) {
					line[0] = p1;
					line[1] = p2;
					n = 2;
					break;
				}
				if(n >= 2) {
					break;
				}
			}

			wxASSERT(n == 2);
			m_canvas->DrawLine(line[0], line[1], 1.0, RED);
		}
	}

	const std::vector<glm::vec2> &pts = p->points;

	m_canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);

	if(p == m_context->GetSelectedPoly()) {
		m_canvas->OutlinePoly(pts.data(), pts.size(), 1.0, GREEN);
	} else {
		m_canvas->OutlinePoly(pts.data(), pts.size(), 1.0, WHITE);
	}

	if(p->GetTexture() != nullptr) {
		m_canvas->TexturePoly(*p, WHITE);
	}
}


IBaseEdit::IBaseEdit(GLCanvas *canvas)
	: m_canvas(canvas),
	  m_context(&canvas->editor),
	  m_view(canvas->view)
{
}


IBaseEdit::~IBaseEdit()
{

}


void IBaseEdit::OnDraw()
{

}


EditorContext::EditorContext(GLCanvas *canvas)
	: m_canvas(canvas),
	  m_name("untitled"),
	  m_hasfile(false),
	  m_state(nullptr)
{
	// setup default layer
	m_layers.emplace_back("default");
	m_curlayer = m_layers.size() - 1;

	LayerList *llist = LayerList::GetInstance();
	llist->SetItemCount(m_layers.size());
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
	GLContext *ctx = GLContext::GetInstance();

	if(m_state != nullptr) {
		ConvexPolygon *spoly = GetSelectedPoly();

		for(EditorLayer &layer : m_layers) {
			for(ConvexPolygon &p : layer.GetPolys()) {
				if(&p != spoly) {
					m_state->DrawPolygon(&p);
				}
			}
			ctx->m_texture.CopyBuffersAndDrawElements();
			ctx->m_solid.CopyBuffers();
			ctx->m_solid.DrawElements();
			ctx->m_texture.ClearBuffers();
			ctx->m_solid.ClearBuffers();
		}

		if(spoly != nullptr) {
			m_state->DrawPolygon(spoly);
		}

		m_state->OnDraw();
	}
}

/* we want our Editor events to be processed before any other event. */
void EnqueueEventHandler(wxWindowBase *window, wxEvtHandler *handler)
{
	wxEvtHandler *last = window->GetEventHandler();

	while(last && last->GetNextHandler() != window) {
		last = last->GetNextHandler();
	}

	last->SetNextHandler(handler);
	handler->SetNextHandler(window);
	handler->SetPreviousHandler(last);
	window->SetPreviousHandler(handler);
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
			EnqueueEventHandler(m_canvas, m_state);
		}
	}
}


ConvexPolygon *EditorContext::FindPoly(glm::vec2 wpos)
{
	EditorLayer *layer = GetSelectedLayer();
	if(layer == nullptr) {
		return nullptr;
	}

	for(ConvexPolygon &p : layer->GetPolys()) {
		if(p.Contains(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


void EditorContext::ResetPolys()
{
	for(EditorLayer &layer : m_layers) {
		layer.GetPolys().clear();
	}

	for(size_t i = 0; i < m_history; i++) {
		ApplyAction(m_actions[i]);
	}

	//poly->PurgePlanes();
	//poly->ResizeAABB();
}


void EditorContext::Undo()
{
	if(m_history == 0) {
		return;
	}

	EditAction &back = m_actions[m_history - 1];

	if(back.base.layer <= 0 || back.base.layer >= m_layers.size()) {
		return;
	}

	ConvexPolygon *poly;
	EditorLayer &layer = m_layers[back.base.layer];
	std::vector<ConvexPolygon> &polys = layer.GetPolys();

	m_history--;

	switch(back.base.type) {
	case EditActionType::TEXTURE:
	case EditActionType::LINE:
	case EditActionType::DEL:
		ResetPolys();
		break;
	case EditActionType::MOVE:
		poly = &polys[back.base.poly];
		poly->Offset(-back.move.delta);
		break;
	case EditActionType::SCALE:
		poly = &polys[back.base.poly];
		poly->Scale(back.scale.origin, back.scale.denom, back.scale.numer);
		break;
	case EditActionType::RECT:
		if(m_selected == back.base.poly) {
			m_selected = -1;
		}
		polys.erase(polys.begin() + back.base.poly);
		break;
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}

ConvexPolygon *EditorContext::ApplyAction(const EditAction &action)
{
	ConvexPolygon *poly = nullptr;
	if(action.base.layer <= 0 || action.base.layer >= m_layers.size()) {
		return nullptr;
	}

	EditorLayer &layer = m_layers[action.base.layer];
	std::vector<ConvexPolygon> &polys = layer.GetPolys();

	switch(action.base.type) {
	case EditActionType::LINE:
		poly = &polys[action.base.poly];
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType::MOVE:
		poly = &polys[action.base.poly];
		poly->Offset(action.move.delta);
		break;
	case EditActionType::SCALE:
		poly = &polys[action.base.poly];
		poly->Scale(action.scale.origin, action.scale.numer, action.scale.denom);
		break;
	case EditActionType::RECT:
		polys.push_back(action.rect.rect);
		poly = &polys.back();
		break;
	case EditActionType::DEL:
		polys.erase(polys.begin() + action.base.poly);
		poly = nullptr;
		return nullptr;
		break;
	case EditActionType::TEXTURE:
		poly = &polys[action.base.poly];
		poly->texindex = action.texture.index;
		poly->texscale = action.texture.scale;
		break;
	}

	wxASSERT(poly);
	wxASSERT_MSG(action.base.poly == poly - &polys.front(),
		"Polygon indices out of order.");

	return poly;
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

	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(m_textures.size());
	tlist->SetSelected(m_textures.size() - 1);
	tlist->Refresh();
}


void EditorContext::Redo()
{
	if(m_history >= m_actions.size()) {
		return;
	}

	m_history++;
	EditAction &last = m_actions[m_history - 1];
	ApplyAction(last);

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}


void EditorContext::AppendAction(EditAction action)
{
	EditorLayer *layer = GetSelectedLayer();
	ConvexPolygon *poly = GetSelectedPoly();

	std::vector<ConvexPolygon> &polys = layer->GetPolys();

	switch(action.base.type) {
	case EditActionType::LINE:
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType::MOVE:
		if(action.move.delta.x == 0 && action.move.delta.y == 0) {
			return;
		}
		poly->Offset(action.move.delta);
		break;
	case EditActionType::SCALE:
		poly->Scale(action.scale.origin, action.scale.numer, action.scale.denom);
		break;
	case EditActionType::RECT:
		polys.push_back(action.rect.rect);
		m_selected = polys.size() - 1; 
		break;
	case EditActionType::DEL:
		polys.erase(polys.begin() + m_selected);
		break;
	case EditActionType::TEXTURE:
		poly->texindex = action.texture.index;
		poly->texscale = action.texture.scale;
		break;
	}

	action.base.poly = m_selected;

	if(action.base.type == EditActionType::DEL) {
		/* deselect on delete */
		m_selected = -1;
	}

	if(action.base.type == EditActionType::RECT || action.base.type == EditActionType::DEL) {
		LayerList *llist = LayerList::GetInstance();
		llist->Refresh();
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	if(!m_actions.empty() && m_history) {
		EditAction &back = m_actions[m_history - 1];
		if(back.base.type == EditActionType::MOVE && action.base.type == EditActionType::MOVE) {
			/* we don't want to spam a move action for each pixel moved */
			if(back.base.poly == action.base.poly) {
				back.move.delta = back.move.delta + action.move.delta;

				/* remove if no-op */
				if(back.move.delta.x == 0 && back.move.delta.y == 0) {
					m_actions.pop_back();
					m_history = m_actions.size();
					hlist->SetItemCount(m_actions.size());
				}

				hlist->Refresh(false);
				Save();
				return;
			}
		}

		if(back.base.type == EditActionType::SCALE && action.base.type == EditActionType::SCALE) {
			if(back.base.poly == action.base.poly && back.scale.origin == action.scale.origin) {
				back.scale.denom *= action.scale.denom;
				back.scale.numer *= action.scale.numer;

				glm::i32vec2 g;
				g.x = std::gcd(back.scale.numer.x, back.scale.denom.x);
				g.y = std::gcd(back.scale.numer.y, back.scale.denom.y);
				back.scale.numer /= g;
				back.scale.denom /= g;

				/* remove if no-op */
				if(back.scale.numer == back.scale.denom) {
					m_actions.pop_back();
					m_history = m_actions.size();
					hlist->SetItemCount(m_actions.size());
				}

				hlist->Refresh(false);
				Save();
				return;
			}
		}

		/* don't bother saving repeat texture actions */
		if(back.base.type == EditActionType::TEXTURE && action.base.type == EditActionType::TEXTURE) {
			if(back.base.poly == action.base.poly
			&& action.texture.index == back.texture.index
			&& action.texture.scale == back.texture.scale) {
				return;
			}
		}
	}

	/* remove future */
	while(m_actions.size() > m_history) {
		m_actions.pop_back();
	}

	m_actions.push_back(action);
	m_history = m_actions.size();

	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	Save();

	m_canvas->Refresh();
}
