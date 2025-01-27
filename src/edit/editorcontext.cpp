#include <array>

#include "src/lvledit2d.hpp"
#include "src/glcanvas.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/mainframe.hpp"
#include "src/historylist.hpp"

struct L2dHeader 
{
	uint8_t magic[2];
	uint32_t actions_offset;
	uint32_t actions_size;
	uint32_t actions_index;
};


bool EditorContext::Load(const wxFileName &path)
{
	wxASSERT(path.FileExists());

	m_name = path.GetName();
	m_file = fopen(path.GetFullPath(), "rb");

	L2dHeader hdr;
	rewind(m_file);
	fread(&hdr, sizeof(L2dHeader), 1, m_file);

	if(hdr.magic[0] != 'L' && hdr.magic[1] != '2') {
		fclose(m_file);
		m_file = nullptr;
		return false;
	}

	size_t num_actions = hdr.actions_size / sizeof(EditAction);
	m_actions.resize(num_actions);
	m_history = hdr.actions_index;

	fseek(m_file, hdr.actions_offset, SEEK_SET);
	fread(m_actions.data(), hdr.actions_size, 1, m_file);

	for(size_t i = 0; i < m_history; i++) {
		ApplyAction(m_actions[i]);
	}

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	HistoryList *hlist = mainframe->GetHistoryList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	/* reopen our file for writing */
	freopen(path.GetPath(), "wb", m_file);

	return true;
}


bool EditorContext::Save()
{
	if(m_file == nullptr) {
		return false;
	}

	L2dHeader hdr;
	hdr.magic[0] = 'L';
	hdr.magic[1] = '2';
	hdr.actions_offset = sizeof(L2dHeader);
	hdr.actions_size = m_actions.size() * sizeof(EditAction);
	hdr.actions_index = m_history;

	rewind(m_file);
	fwrite(&hdr, sizeof(L2dHeader), 1, m_file);

	fseek(m_file, hdr.actions_offset, SEEK_SET);
	fwrite(m_actions.data(), hdr.actions_size, 1, m_file);

	return true;
}


bool EditorContext::Save(const wxFileName &path)
{
	wxASSERT(path.GetExt() == "l2d");

	if(m_file != nullptr) {
		fclose(m_file);
	}

	m_name = path.GetName();
	m_file = fopen(path.GetFullPath(), "wb");

	wxASSERT(m_file != nullptr);

	return Save();
}


void IBaseEdit::DrawPolygon(const ConvexPolygon *p)
{
	const Color blue = Color(0.2f, 0.2f, 1.0f, 1.0f);
	const Color red = Color(1.0f, 0.2f, 0.2f, 1.0f);
	const Color white = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color green = Color(0.2, 1.0f, 0.2f, 1.0f);

	if(p == m_context->GetSelectedPoly()) {
		Rect2D aabb = p->GetAABB();
		m_canvas->DrawRect(aabb, Color(0.2f, 0.2f, 1.0f, 1.0f));

		/* inflate aabb to illustrate planes */
		const double outset = -50.0;
		aabb.Inset(outset, outset);

		std::array aabbpts = {
			aabb.GetLeftTop(),
			aabb.GetRightTop(),
			aabb.GetRightBottom(),
			aabb.GetLeftBottom()
		};

		std::vector<Point2D> plane_lines;

		for(const Plane2D &plane : p->GetPlanes()) {
			size_t naabbpts = aabbpts.size();
			for(size_t i = 0; i < naabbpts; i++) {
				Point2D a = aabbpts[i];
				Point2D b = aabbpts[(i + 1) % naabbpts];

				double da = plane.SignedDistance(a);
				double db = plane.SignedDistance(b);

				if(da * db < 0) {
					Point2D isect;
					if(plane.Line(a, b, isect)) {
						plane_lines.push_back(isect);
					}
				}
			}
		}

		if(plane_lines.size() >= 2)
			for(size_t i = 0; i < plane_lines.size() - 1; i += 2) {
				Point2D a = plane_lines[i];
				Point2D b = plane_lines[i + 1];
				m_canvas->DrawLine(a, b, 1.0, red);
			}
	}

	if(p == m_context->GetSelectedPoly()) {
		m_canvas->DrawPolygon(*p, green);
	}
	else {
		m_canvas->DrawPolygon(*p, Color(1.0f, 1.0f, 1.0f, 1.0f));
	}
}


IBaseEdit::IBaseEdit(GLCanvas *canvas)
	: m_canvas(canvas),
	  m_context(&m_canvas->GetEditor()),
	  m_view(m_canvas->GetView())
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
	  m_name("untitled")
{
	m_state = nullptr;
}


EditorContext::~EditorContext()
{
	if(m_state != nullptr) {
		m_canvas->RemoveEventHandler(m_state);
		delete m_state;
	}

	if(m_file != nullptr) {
		Save();
		fclose(m_file);
	}
}


void EditorContext::OnDraw()
{
	if(m_state != nullptr) {

		m_state->OnDraw();

		for(ConvexPolygon &p : m_polys) {
			if(&p != m_selected) {
				m_state->DrawPolygon(&p);
			}
		}

		if(m_selected != nullptr) {
			m_state->DrawPolygon(m_selected);
		}
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
		default:
			break;
		}
		if(m_state != nullptr) {
			EnqueueEventHandler(m_canvas, m_state);
		}
	}
}


ConvexPolygon *EditorContext::SelectPoly(Point2D wpos)
{
	for(ConvexPolygon &p : m_polys) {
		if(p.Contains(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


void EditorContext::ResetPoly(size_t i)
{
	ConvexPolygon *poly = nullptr;

	for(size_t act = 0; act < m_history; act++) {
		const EditAction &action = m_actions[act];
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
			case EditActionType_t::RECT:
				m_polys[i] = action.rect.rect;
				poly = &m_polys[i];
				break;
			}
		}
	}

	poly->PurgePlanes();
	poly->ResizeAABB();
}


void EditorContext::Undo()
{
	if(m_history == 0) {
		return;
	}

	EditAction &back = LastAction();
	ConvexPolygon &poly = m_polys[back.base.poly];

	m_history--;

	switch(back.base.type) {
	case EditActionType_t::LINE:
		ResetPoly(back.base.poly);
		break;
	case EditActionType_t::MOVE:
		poly.MoveBy(-back.move.delta);
		break;
	case EditActionType_t::RECT:
		if(m_selected == &m_polys[back.base.poly]) {
			m_selected = nullptr;
		}
		m_polys.erase(m_polys.begin() + back.base.poly);
		break;
	}

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	HistoryList *hlist = mainframe->GetHistoryList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}

ConvexPolygon *EditorContext::ApplyAction(const EditAction &action)
{
	ConvexPolygon *poly = nullptr;

	switch(action.base.type) {
	case EditActionType_t::LINE:
		poly = &m_polys[action.base.poly];
		poly->Slice(action.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType_t::MOVE:
		poly = &m_polys[action.base.poly];
		poly->MoveBy(action.move.delta);
		break;
	case EditActionType_t::RECT:
		m_polys.push_back(action.rect.rect);
		poly = &m_polys.back();
		break;
	}

	wxASSERT_MSG(action.base.poly == poly - &m_polys.front(),
		"Polygon indices out of order.");

	return poly;
}

void EditorContext::Redo()
{
	if(m_history >= m_actions.size()) {
		return;
	}

	m_history++;
	EditAction &last = LastAction();
	ApplyAction(last);

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	HistoryList *hlist = mainframe->GetHistoryList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}


void EditorContext::AppendAction(EditAction action)
{
	switch(action.base.type) {
	case EditActionType_t::LINE:
		m_selected->Slice(action.line.plane);
		m_selected->PurgePlanes();
		m_selected->ResizeAABB();
		break;
	case EditActionType_t::MOVE:
		m_selected->MoveBy(action.move.delta);
		break;
	case EditActionType_t::RECT:
		m_polys.push_back(action.rect.rect);
		m_selected = &m_polys.back();
		break;
	}

	action.base.poly = m_selected - &m_polys.front();

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	HistoryList *hlist = mainframe->GetHistoryList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	if(!m_actions.empty() && m_history) {
		EditAction &back = LastAction();
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
	while(m_actions.size() > m_history) {
		m_actions.pop_back();
	}

	m_actions.push_back(action);
	m_history = m_actions.size();

	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	Save();
}
