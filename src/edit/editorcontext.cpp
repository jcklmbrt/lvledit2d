#include <array>
#include <cfloat>
#include <cstdio>
#include <wx/debug.h>
#include <wx/listbase.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>

#include "src/gl/glcanvas.hpp"
#include "src/geometry.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/edit/textureedit.hpp"
#include "src/gl/texture.hpp"
#include "src/historylist.hpp"
#include "src/texturepanel.hpp"

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

/* stripped down version of GLTexture */
struct TexInfo 
{
	char name[MAX_TEXTURE_NAME];
	uint32_t width;
	uint32_t height;
	uint8_t pixelwidth;
	uint32_t dataoffset;
};

bool EditorContext::Load(const wxFileName &filename)
{
	wxASSERT(filename.FileExists());

	name = filename.GetName();
	path = filename.GetFullPath();
	has_file = true;
	FILE *file = fopen(path.c_str(), "rb");

	L2dHeader hdr;
	fseek(file, 0, SEEK_SET);
	fread(&hdr, sizeof(L2dHeader), 1, file);

	if(hdr.magic[0] != 'L' && hdr.magic[1] != '2') {
		fclose(file);
		file = nullptr;
		return false;
	}

	size_t num_actions = hdr.actions_size / sizeof(EditAction);
	actions.resize(num_actions);
	history = hdr.actions_index;

	fseek(file, hdr.actions_offset, SEEK_SET);
	fread(actions.data(), hdr.actions_size, 1, file);

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

		textures.push_back(texture);
	}

	ResetPolys();

	wxPoint2DDouble d;
	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(textures.size());
	tlist->Refresh();

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(actions.size());
	hlist->Refresh();

	canvas->Refresh(true);

	return true;
}


bool EditorContext::Save()
{
	if(!has_file) {
		return false;
	}

	FILE *file = fopen(path.c_str(), "wb");

	std::vector<unsigned char> texdata;
	std::vector<TexInfo> texinfo;

	for(GLTexture &texture : textures) {
		TexInfo &info = texinfo.emplace_back();
		strncpy(info.name, texture.name, MAX_TEXTURE_NAME);
		info.width = texture.width;
		info.height = texture.height;
		info.pixelwidth = texture.pixelwidth;

		info.dataoffset = texdata.size();
		size_t nbytes = texture.width * texture.height * texture.pixelwidth;
		texdata.resize(texdata.size() + nbytes);
		memcpy(texdata.data() + info.dataoffset, texture.data, nbytes);
	}

	L2dHeader hdr;
	hdr.magic[0] = 'L';
	hdr.magic[1] = '2';
	hdr.actions_offset = sizeof(L2dHeader);
	hdr.actions_size = actions.size() * sizeof(EditAction);
	hdr.actions_index = history;
	hdr.texinfo_offset = hdr.actions_offset + hdr.actions_size;
	hdr.texinfo_size = texinfo.size() * sizeof(TexInfo);
	hdr.texdata_offset = hdr.texinfo_offset + hdr.texinfo_size;
	hdr.texdata_size = texdata.size();

	fseek(file, 0, SEEK_SET);
	fwrite(&hdr, sizeof(L2dHeader), 1, file);

	fseek(file, hdr.actions_offset, SEEK_SET);
	fwrite(actions.data(), hdr.actions_size, 1, file);

	fseek(file, hdr.texinfo_offset, SEEK_SET);
	fwrite(texinfo.data(), hdr.texinfo_size, 1, file);

	fseek(file, hdr.texdata_offset, SEEK_SET);
	fwrite(texdata.data(), hdr.texdata_size, 1, file);

	return true;
}


bool EditorContext::Save(const wxFileName &filename)
{
	wxASSERT(filename.GetExt() == "l2d");

	name = filename.GetName();
	path = filename.GetFullPath();
	has_file = true;

	return Save();
}


void IBaseEdit::DrawPolygon(const ConvexPolygon *p)
{
	if(p == context->GetSelectedPoly()) {
		Rect2D aabb = p->aabb;
		canvas->OutlineRect(aabb, 1.0f, BLUE);

		/* inflate aabb to illustrate planes */
		aabb.Inset(-100, -100);

		std::array aabbpts = {
			aabb.mins,
			aabb.GetRightTop(),
			aabb.maxs,
			aabb.GetLeftBottom()
		};

		for(const Plane2D &plane : p->planes) {
			Point2D line[2];
			int n = 0;
			size_t naabbpts = aabbpts.size();
			for(size_t i = 0; i < naabbpts; i++) {
				Point2D a = aabbpts[i];
				Point2D b = aabbpts[(i + 1) % naabbpts];

				float da = plane.SignedDistance(a);
				float db = plane.SignedDistance(b);

				if(fabs(da) <= FLT_EPSILON) {
					/* a is on the plane */
					line[n++] = a;
				} else if(fabs(db) <= FLT_EPSILON) {
					/* next point will be on the plane */
				} else if(da * db <= 0.0) {
					/* a point between a and b intersects with the plane */
					Point2D isect;
					if(plane.Line(a, b, isect)) {
						line[n++] = isect;
					}
					
				}
			}

			wxASSERT(n == 2);
			canvas->DrawLine(line[0], line[1], 1.0, RED);
		}
	}

	const std::vector<Point2D> &pts = p->points;

	canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);

	if(p == context->GetSelectedPoly()) {
		canvas->OutlinePoly(pts.data(), pts.size(), 1.0, GREEN);
	} else {
		canvas->OutlinePoly(pts.data(), pts.size(), 1.0, WHITE);
	}

	if(p->GetTexture() != nullptr) {
		canvas->TexturePoly(*p, WHITE);
	}
}


IBaseEdit::IBaseEdit(GLCanvas *canvas)
	: canvas(canvas),
	  context(&canvas->editor),
	  view(canvas->view)
{
}


IBaseEdit::~IBaseEdit()
{

}


void IBaseEdit::OnDraw()
{

}


EditorContext::EditorContext(GLCanvas *canvas)
	: canvas(canvas),
	  name("untitled"),
	  has_file(false),
	  state(nullptr)
{
}


EditorContext::~EditorContext()
{
	if(state != nullptr) {
		canvas->RemoveEventHandler(state);
		delete state;
	}

	if(has_file) {
		Save();
	}
}


void EditorContext::OnDraw()
{
	if(state != nullptr) {

		ConvexPolygon *spoly = GetSelectedPoly();

		for(ConvexPolygon &p : polys) {
			if(&p != spoly) {
				state->DrawPolygon(&p);
			}
		}

		if(spoly != nullptr) {
			state->DrawPolygon(spoly);
		}

		state->OnDraw();
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
	if(state != nullptr) {
		if(state != nullptr) {
			canvas->RemoveEventHandler(state);
			delete state;
		}
		state = nullptr;
		canvas->Refresh(false);
	}

	if(state == nullptr) {
		switch(id) {
		case ToolBar::ID::SELECT:
			state = new SelectionEdit(canvas);
			break;
		case ToolBar::ID::QUAD:
			state = new RectangleEdit(canvas);
			break;
		case ToolBar::ID::LINE:
			state = new LineEdit(canvas);
			break;
		case ToolBar::ID::TEXTURE:
			state = new TextureEdit(canvas);
			break;
		default:
			break;
		}
		if(state != nullptr) {
			EnqueueEventHandler(canvas, state);
		}
	}
}


ConvexPolygon *EditorContext::FindPoly(Point2D wpos)
{
	for(ConvexPolygon &p : polys) {
		if(p.Contains(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


void EditorContext::ResetPolys()
{
	polys.clear();

	for(size_t i = 0; i < history; i++) {
		ApplyAction(actions[i]);
	}

	//poly->PurgePlanes();
	//poly->ResizeAABB();
}


void EditorContext::Undo()
{
	if(history == 0) {
		return;
	}

	EditAction &back = actions[history - 1];
	ConvexPolygon &poly = polys[back.base.poly];

	history--;

	switch(back.base.type) {
	case EditActionType::TEXTURE:
	case EditActionType::LINE:
	case EditActionType::DELETE:
		ResetPolys();
		break;
	case EditActionType::TRANS:
		poly.Transform(inverse(back.trans.matrix));
		break;
	case EditActionType::RECT:
		if(selected == back.base.poly) {
			selected = -1;
		}
		polys.erase(polys.begin() + back.base.poly);
		break;
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(actions.size());
	hlist->Refresh();
}

ConvexPolygon *EditorContext::ApplyAction(const EditAction &action)
{
	ConvexPolygon *poly = nullptr;

	switch(action.base.type) {
	case EditActionType::LINE:
		poly = &polys[action.base.poly];
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType::TRANS:
		poly = &polys[action.base.poly];
		poly->Transform(action.trans.matrix);
		break;
	case EditActionType::RECT:
		polys.push_back(action.rect.rect);
		poly = &polys.back();
		break;
	case EditActionType::DELETE:
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

	for(GLTexture &t : textures) {
		if(texture == t) {
			return;
		}
	}

	textures.push_back(texture);

	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(textures.size());
	tlist->selected = textures.size() - 1;
	tlist->Refresh();
}


void EditorContext::Redo()
{
	if(history >= actions.size()) {
		return;
	}

	history++;
	EditAction &last = actions[history - 1];
	ApplyAction(last);

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(actions.size());
	hlist->Refresh();
}


void EditorContext::AppendAction(EditAction action)
{
	ConvexPolygon *poly = GetSelectedPoly();

	switch(action.base.type) {
	case EditActionType::LINE:
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType::TRANS:
		poly->Transform(action.trans.matrix);
		break;
	case EditActionType::RECT:
		polys.push_back(action.rect.rect);
		selected = polys.size() - 1; 
		break;
	case EditActionType::DELETE:
		polys.erase(polys.begin() + selected);
		break;
	case EditActionType::TEXTURE:
		poly->texindex = action.texture.index;
		poly->texscale = action.texture.scale;
		break;
	}

	action.base.poly = selected;

	if(action.base.type == EditActionType::DELETE) {
		/* deselect on delete */
		selected = -1;
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(actions.size());
	hlist->Refresh();

	if(!actions.empty() && history) {
		EditAction &back = actions[history - 1];
		if(back.base.type == EditActionType::TRANS && action.base.type == EditActionType::TRANS) {
			/* we don't want to spam a move action for each pixel moved */
			if(back.base.poly == action.base.poly) {
				back.trans.matrix = back.trans.matrix * action.trans.matrix;
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
	while(actions.size() > history) {
		actions.pop_back();
	}

	actions.push_back(action);
	history = actions.size();

	hlist->SetItemCount(actions.size());
	hlist->Refresh();

	Save();

	canvas->Refresh();
}
