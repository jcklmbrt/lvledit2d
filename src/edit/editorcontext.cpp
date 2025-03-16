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

struct Lump
{
	uint32_t ofs, size;
};

struct L2dHeader 
{
	uint8_t magic[2];
	Lump actions;
	Lump texinfo;
	Lump texdata;
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

	unsigned char *action_data = new unsigned char[hdr.actions.size];

	fseek(file, hdr.actions.ofs, SEEK_SET);
	fread(action_data, hdr.actions.size, 1, file);
	m_actions.Deserialize(action_data, hdr.actions.size);

	delete[] action_data;

	std::vector<TexInfo> texinfo;
	texinfo.resize(hdr.texinfo.size / sizeof(TexInfo));
	fseek(file, hdr.texinfo.ofs, SEEK_SET);
	fread(texinfo.data(), hdr.texinfo.size, 1, file);

	std::vector<unsigned char> texdata;
	texdata.resize(hdr.texdata.size);
	fseek(file, hdr.texdata.ofs, SEEK_SET);
	fread(texdata.data(), hdr.texdata.size, 1, file);

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
	hdr.actions.ofs = sizeof(L2dHeader);
	hdr.actions.size = m_actions.BinSize();
	hdr.texinfo.ofs = hdr.actions.ofs + hdr.actions.size;
	hdr.texinfo.size = texinfo.size() * sizeof(TexInfo);
	hdr.texdata.ofs = hdr.texinfo.ofs + hdr.texinfo.size;
	hdr.texdata.size = texdata.size();

	fseek(file, 0, SEEK_SET);
	fwrite(&hdr, sizeof(L2dHeader), 1, file);

	unsigned char *action_data = new unsigned char[hdr.actions.size];
	m_actions.Seralize(action_data, hdr.actions.size);

	fseek(file, hdr.actions.ofs, SEEK_SET);
	fwrite(action_data, hdr.actions.size, 1, file);

	delete[] action_data;

	fseek(file, hdr.texinfo.ofs, SEEK_SET);
	fwrite(texinfo.data(), hdr.texinfo.size, 1, file);

	fseek(file, hdr.texdata.ofs, SEEK_SET);
	fwrite(texdata.data(), hdr.texdata.size, 1, file);

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
	if(p == m_edit.GetSelectedPoly()) {
		Rect2D aabb = p->GetAABB();
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

		for(const Plane2D &plane : p->GetPlanes()) {
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

	const std::vector<glm::vec2> &pts = p->GetPoints();

	m_canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);

	if(p == m_edit.GetSelectedPoly()) {
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
	  m_edit(canvas->GetEditor()),
	  m_view(canvas->GetView())
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
	if(m_state != nullptr) {
		ConvexPolygon *spoly = GetSelectedPoly();

		for(EditorLayer &layer : m_layers) {
			for(ConvexPolygon &p : layer.GetPolys()) {
				if(&p != spoly) {
					m_state->DrawPolygon(&p);
				}
			}
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

ConvexPolygon *EditorContext::ApplyAction(const ActData &act)
{
	ConvexPolygon *poly = nullptr;
	if(act.layer < 0 || act.layer >= m_layers.size()) {
		return nullptr;
	}

	EditorLayer &layer = m_layers[act.layer];
	std::vector<ConvexPolygon> &polys = layer.GetPolys();

	switch(act.type) {
	case ActType::LINE:
		poly = &polys[act.poly];
		poly->Slice(act.line);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case ActType::MOVE:
		poly = &polys[act.poly];
		poly->Offset(act.move);
		break;
	case ActType::SCALE:
		poly = &polys[act.poly];
		poly->Scale(act.scale.origin, act.scale.numer, act.scale.denom);
		break;
	case ActType::RECT:
		polys.push_back(act.rect);
		poly = &polys.back();
		break;
	case ActType::DEL:
		polys.erase(polys.begin() + act.poly);
		poly = nullptr;
		return nullptr;
		break;
	case ActType::TEXTURE:
		poly = &polys[act.poly];
		poly->SetTexture(act.texture.index, act.texture.scale);
		break;
	}
	wxASSERT(poly);
	wxASSERT_MSG(act.poly == poly - &polys.front(),
		"Polygon indices out of order.");

	return poly;
}


ConvexPolygon *EditorContext::UndoAction(const ActData &act)
{
	ConvexPolygon *poly = nullptr;
	EditorLayer &layer = m_layers[act.layer];
	std::vector<ConvexPolygon> &polys = layer.GetPolys();

	switch(act.type) {
	case ActType::TEXTURE:
	case ActType::LINE:
	case ActType::DEL:
		ResetPolys();
		break;
	case ActType::MOVE:
		poly = &polys[act.poly];
		glm::i32vec2 move = act.move;
		poly->Offset(-move);
		break;
	case ActType::SCALE:
		poly = &polys[act.poly];
		poly->Scale(act.scale.origin, act.scale.denom, act.scale.numer);
		break;
	case ActType::RECT:
		if(m_selected == act.poly) {
			m_selected = -1;
		}
		polys.erase(polys.begin() + act.poly);
		break;
	}
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
	EditorLayer *layer = GetSelectedLayer();
	std::vector<ConvexPolygon> &polys = layer->GetPolys();

	m_selected = polys.size();

	m_actions.AddAction(rect, polys.size(), m_curlayer);

	ActData back;
	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}

void EditorContext::AddAction(const ActLine &line)
{
	ConvexPolygon *poly = GetSelectedPoly();
	if(poly == nullptr) {
		return;
	}

	poly->Slice(line);
	poly->PurgePlanes();
	poly->ResizeAABB();

	m_actions.AddAction(line, m_selected, m_curlayer);

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

	if(back.type == ActType::MOVE && back.poly == m_selected) {
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
		m_actions.AddAction(move, m_selected, m_curlayer);
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
	if(!m_actions.IsEmpty() && m_actions.HistoryIndex()) {
		m_actions.GetBack(back);
		if(back.type == ActType::SCALE &&
		   back.poly == m_selected &&
		   back.layer == m_curlayer &&
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
		m_actions.AddAction(scale, m_selected, m_curlayer);
	}

	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}

void EditorContext::AddAction(const ActTexture &texture)
{
	ConvexPolygon *poly = GetSelectedPoly();
	if(poly == nullptr) {
		return;
	}

	poly->SetTexture(texture.index, texture.scale);

	ActData back;
	if(!m_actions.IsEmpty() && m_actions.HistoryIndex()) {
		m_actions.GetBack(back);
		/* don't bother saving repeat texture actions */
		if(back.type == ActType::TEXTURE) {
			if(back.poly == m_selected
				&& texture.index == back.texture.index
				&& texture.scale == back.texture.scale) {
				return;
			}
		}
	}

	m_actions.AddAction(texture, m_selected, m_curlayer);
	Save();
	m_canvas->Refresh();
}

void EditorContext::AddDelete()
{
	LayerList *llist = LayerList::GetInstance();
	llist->Refresh();

	m_actions.AddDelete(m_selected, m_curlayer);

	m_selected = -1;

	ActData back;
	m_actions.GetBack(back);
	ApplyAction(back);

	Save();
	m_canvas->Refresh();
}