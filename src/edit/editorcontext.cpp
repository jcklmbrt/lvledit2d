#include <array>
#include <cfloat>
#include <wx/debug.h>
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
	char Name[MAX_TEXTURE_NAME];
	uint32_t Width;
	uint32_t Height;
	uint8_t PixelWidth;
	uint32_t DataOffset;
};

bool EditorContext::Load(const wxFileName &path)
{
	wxASSERT(path.FileExists());

	Name = path.GetName();
	File = fopen(path.GetFullPath(), "rb");

	L2dHeader hdr;
	rewind(File);
	fread(&hdr, sizeof(L2dHeader), 1, File);

	if(hdr.magic[0] != 'L' && hdr.magic[1] != '2') {
		fclose(File);
		File = nullptr;
		return false;
	}

	size_t num_actions = hdr.actions_size / sizeof(EditAction);
	Actions.resize(num_actions);
	History = hdr.actions_index;

	fseek(File, hdr.actions_offset, SEEK_SET);
	fread(Actions.data(), hdr.actions_size, 1, File);

	for(size_t i = 0; i < History; i++) {
		ApplyAction(Actions[i]);
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(Actions.size());
	hlist->Refresh();

	/* reopen our file for writing */
	freopen(path.GetFullPath(), "wb", File);

	return true;
}


bool EditorContext::Save()
{
	if(File == nullptr) {
		return false;
	}

	L2dHeader hdr;
	hdr.magic[0] = 'L';
	hdr.magic[1] = '2';
	hdr.actions_offset = sizeof(L2dHeader);
	hdr.actions_size = Actions.size() * sizeof(EditAction);
	hdr.actions_index = History;

	rewind(File);
	fwrite(&hdr, sizeof(L2dHeader), 1, File);

	fseek(File, hdr.actions_offset, SEEK_SET);
	fwrite(Actions.data(), hdr.actions_size, 1, File);

	return true;
}


bool EditorContext::Save(const wxFileName &path)
{
	wxASSERT(path.GetExt() == "l2d");

	if(File != nullptr) {
		fclose(File);
	}

	Name = path.GetName();
	File = fopen(path.GetFullPath(), "wb");

	wxASSERT(File != nullptr);

	return Save();
}


void IBaseEdit::DrawPolygon(const ConvexPolygon *p)
{
	if(p == Context->GetSelectedPoly()) {
		Rect2D aabb = p->GetAABB();
		Canvas->OutlineRect(aabb, 1.0f, BLUE);

		/* inflate aabb to illustrate planes */
		aabb.Inset(-100, -100);

		std::array aabbpts = {
			aabb.GetLeftTop(),
			aabb.GetRightTop(),
			aabb.GetRightBottom(),
			aabb.GetLeftBottom()
		};

		for(const Plane2D &plane : p->GetPlanes()) {
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
			Canvas->DrawLine(line[0], line[1], 1.0, RED);
		}
	}

	const std::vector<Point2D> &pts = p->GetPoints();

	Canvas->OutlinePoly(pts.data(), pts.size(), 3.0, BLACK);

	if(p == Context->GetSelectedPoly()) {
		Canvas->OutlinePoly(pts.data(), pts.size(), 1.0, GREEN);
		for(const Point2D &pt : pts) {
			Canvas->DrawPoint(pt, WHITE);
		}
	}
	else {
		Canvas->OutlinePoly(pts.data(), pts.size(), 1.0, WHITE);
	}

	if(p->GetTexture() != nullptr) {
		Canvas->TexturePoly(*p, WHITE);
	}
}


IBaseEdit::IBaseEdit(GLCanvas *canvas)
	: Canvas(canvas),
	  Context(&Canvas->Editor),
	  View(Canvas->ViewMatrix)
{
}


IBaseEdit::~IBaseEdit()
{

}


void IBaseEdit::OnDraw()
{

}


EditorContext::EditorContext(GLCanvas *canvas)
	: Canvas(canvas),
	  Name("untitled")
{
	State = nullptr;
}


EditorContext::~EditorContext()
{
	if(State != nullptr) {
		Canvas->RemoveEventHandler(State);
		delete State;
	}

	if(File != nullptr) {
		Save();
		fclose(File);
	}
}


void EditorContext::OnDraw()
{
	if(State != nullptr) {

		ConvexPolygon *Selected = GetSelectedPoly();

		for(ConvexPolygon &p : Polygons) {
			if(&p != Selected) {
				State->DrawPolygon(&p);
			}
		}

		if(Selected != nullptr) {
			State->DrawPolygon(Selected);
		}

		State->OnDraw();
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
	if(State != nullptr) {
		if(State != nullptr) {
			Canvas->RemoveEventHandler(State);
			delete State;
		}
		State = nullptr;
		Canvas->Refresh(false);
	}

	if(State == nullptr) {
		switch(id) {
		case ToolBar::ID::SELECT:
			State = new SelectionEdit(Canvas);
			break;
		case ToolBar::ID::QUAD:
			State = new RectangleEdit(Canvas);
			break;
		case ToolBar::ID::LINE:
			State = new LineEdit(Canvas);
			break;
		case ToolBar::ID::TEXTURE:
			State = new TextureEdit(Canvas);
			break;
		default:
			break;
		}
		if(State != nullptr) {
			EnqueueEventHandler(Canvas, State);
		}
	}
}


ConvexPolygon *EditorContext::SelectPoly(Point2D wpos)
{
	for(ConvexPolygon &p : Polygons) {
		if(p.Contains(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


void EditorContext::ResetPoly(size_t i)
{
	ConvexPolygon *poly = nullptr;

	for(size_t act = 0; act < History; act++) {
		const EditAction &action = Actions[act];
		if(action.base.poly == i) {
			switch(action.base.type) {
			case EditActionType_t::LINE:
				wxASSERT(poly);
				poly->Slice(action.line.plane);
				break;
			case EditActionType_t::MOVE:
				wxASSERT(poly);
				poly->MoveBy(action.move.delta);
				break;
			case EditActionType_t::TEXTURE:
				wxASSERT(poly);
				poly->SetTexture(action.texture.Index,
						 action.texture.Scale);
				break;
			case EditActionType_t::RECT:
				Polygons[i] = action.rect.rect;
				poly = &Polygons[i];
				break;
			}
		}
	}

	poly->PurgePlanes();
	poly->ResizeAABB();
}


void EditorContext::Undo()
{
	if(History == 0) {
		return;
	}

	EditAction &back = Actions[History - 1];
	ConvexPolygon &poly = Polygons[back.base.poly];

	History--;

	switch(back.base.type) {
	case EditActionType_t::TEXTURE:
	case EditActionType_t::LINE:
		ResetPoly(back.base.poly);
		break;
	case EditActionType_t::MOVE:
		poly.MoveBy(-back.move.delta);
		break;
	case EditActionType_t::RECT:
		if(SelectedPolygon == back.base.poly) {
			SelectedPolygon = -1;
		}
		Polygons.erase(Polygons.begin() + back.base.poly);
		break;
	}

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(Actions.size());
	hlist->Refresh();
}

ConvexPolygon *EditorContext::ApplyAction(const EditAction &action)
{
	ConvexPolygon *poly = nullptr;

	switch(action.base.type) {
	case EditActionType_t::LINE:
		poly = &Polygons[action.base.poly];
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType_t::MOVE:
		poly = &Polygons[action.base.poly];
		poly->MoveBy(action.move.delta);
		break;
	case EditActionType_t::RECT:
		Polygons.push_back(action.rect.rect);
		poly = &Polygons.back();
		break;
	case EditActionType_t::TEXTURE:
		poly = &Polygons[action.base.poly];
		poly->SetTexture(action.texture.Index,
				 action.texture.Scale);
	}

	wxASSERT_MSG(action.base.poly == poly - &Polygons.front(),
		"Polygon indices out of order.");

	return poly;
}


void EditorContext::AddTexture(const wxFileName &filename)
{
	GLTexture NewTexture;
	LoadTextureFromFile(&NewTexture, filename);

	for(GLTexture &T : Textures) {
		if(EqualTextures(&NewTexture, &T)) {
			return;
		}
	}

	Textures.push_back(NewTexture);

	TextureList *tlist = TextureList::GetInstance();
	tlist->SetItemCount(Textures.size());
	tlist->Refresh();
}


void EditorContext::Redo()
{
	if(History >= Actions.size()) {
		return;
	}

	History++;
	EditAction &last = Actions[History - 1];
	ApplyAction(last);

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(Actions.size());
	hlist->Refresh();
}


void EditorContext::AppendAction(EditAction action)
{
	ConvexPolygon *Selected = GetSelectedPoly();

	switch(action.base.type) {
	case EditActionType_t::LINE:
		Selected->Slice(action.line.plane);
		Selected->PurgePlanes();
		Selected->ResizeAABB();
		break;
	case EditActionType_t::MOVE:
		Selected->MoveBy(action.move.delta);
		break;
	case EditActionType_t::RECT:
		Polygons.push_back(action.rect.rect);
		SelectedPolygon = Polygons.size() - 1; 
		break;
	case EditActionType_t::TEXTURE:
		Selected->SetTexture(action.texture.Index,
				     action.texture.Scale);
		break;
	}

	action.base.poly = SelectedPolygon;

	HistoryList *hlist = HistoryList::GetInstance();
	hlist->SetItemCount(Actions.size());
	hlist->Refresh();

	if(!Actions.empty() && History) {
		EditAction &back = Actions[History - 1];
		if(back.base.type == EditActionType_t::MOVE && action.base.type == EditActionType_t::MOVE) {
			/* we don't want to spam a move action for each pixel moved */
			if(back.base.poly == action.base.poly) {
				back.move.delta += action.move.delta;
				hlist->Refresh(false);
				Save();
				return;
			}
		}
	}

	/* remove future */
	while(Actions.size() > History) {
		Actions.pop_back();
	}

	Actions.push_back(action);
	History = Actions.size();

	hlist->SetItemCount(Actions.size());
	hlist->Refresh();

	Save();
}
