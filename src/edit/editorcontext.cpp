#include <array>
#include <wx/debug.h>
#include "src/lvledit2d.hpp"
#include "src/convexpolygon.hpp"
#include "src/drawpanel.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/mainframe.hpp"
#include "src/historylist.hpp"


void IBaseEdit::DrawPolygon(wxPaintDC &dc, const ConvexPolygon *p)
{
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	if(p == m_context->GetSelectedPoly()) {
		wxRect2DDouble aabb = p->GetAABB();
		wxRect s_aabb;
		s_aabb.SetLeftTop(m_panel->WorldToScreen(aabb.GetLeftTop()));
		s_aabb.SetRightBottom(m_panel->WorldToScreen(aabb.GetRightBottom()));

		dc.SetPen(wxPen(*wxBLUE, 1));
		dc.DrawRectangle(s_aabb);

		/* inflate aabb to illustrate planes */
		const double outset = -50.0;
		aabb.Inset(outset, outset);

		std::array aabbpts = {
			aabb.GetLeftTop(),
			aabb.GetRightTop(),
			aabb.GetRightBottom(),
			aabb.GetLeftBottom()
		};

		std::vector<wxPoint> plane_lines;

		for(const Plane2D &plane : p->GetPlanes()) {
			size_t naabbpts = aabbpts.size();
			for(size_t i = 0; i < naabbpts; i++) {
				wxPoint2DDouble a = aabbpts[i];
				wxPoint2DDouble b = aabbpts[(i + 1) % naabbpts];

				double da = plane.SignedDistance(a);
				double db = plane.SignedDistance(b);

				if(da * db < 0) {
					wxPoint2DDouble isect;
					if(plane.Line(a, b, isect)) {
						wxPoint spt = m_panel->WorldToScreen(isect);
						plane_lines.push_back(spt);
					}
				}
			}
		}

		dc.SetPen(wxPen(*wxRED, 1));
		if(plane_lines.size() >= 2)
			for(size_t i = 0; i < plane_lines.size() - 1; i += 2) {
				wxPoint a = plane_lines[i];
				wxPoint b = plane_lines[i + 1];
				dc.DrawLine(a, b);
			}
	}

	const std::vector<wxPoint2DDouble> &points = p->GetPoints();
	size_t npoints = points.size();
	wxPoint *s_points = new wxPoint[npoints];

	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble point = points[i];
		s_points[i] = m_panel->WorldToScreen(point);
	}

	dc.SetPen(wxPen(*wxBLACK, 3));
	dc.DrawPolygon(npoints, s_points);

	if(p == m_context->GetSelectedPoly()) {
		dc.SetPen(wxPen(*wxGREEN, 1));
	}
	else {
		dc.SetPen(wxPen(*wxWHITE, 1));
	}

	dc.DrawPolygon(npoints, s_points);

	if(p == m_context->GetSelectedPoly()) {
		for(size_t i = 0; i < npoints; i++) {
			m_panel->DrawPoint(dc, s_points[i], wxWHITE);
		}
	}

	delete[] s_points;
}


IBaseEdit::IBaseEdit(DrawPanel *panel)
	: m_panel(panel)
{
	m_context = &m_panel->GetEditor();
}


IBaseEdit::~IBaseEdit()
{

}


DrawPanel *IBaseEdit::GetPanel()
{
	return m_panel;
}


EditorContext::EditorContext(DrawPanel *parent)
	: m_parent(parent)
{
	Bind(wxEVT_PAINT, &EditorContext::OnPaint, this);
	m_state = nullptr;
}


EditorContext::~EditorContext()
{
	if(m_state != nullptr) {
		m_parent->RemoveEventHandler(m_state);
		delete m_state;
	}
}


void EditorContext::OnPaint(wxPaintEvent &e)
{
	e.Skip();
	wxPaintDC dc(m_parent);

	if(m_state != nullptr) {
		for(ConvexPolygon &p : m_polys) {
			if(&p != m_selected) {
				m_state->DrawPolygon(dc, &p);
			}
		}

		if(m_selected != nullptr) {
			m_state->DrawPolygon(dc, m_selected);
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
			m_parent->RemoveEventHandler(m_state);
			delete m_state;
		}
		m_state = nullptr;
		m_parent->Refresh(false);
	}

	if(m_state == nullptr) {
		switch(id) {
		case ToolBar::ID::SELECT:
			m_state = new SelectionEdit(m_parent);
			break;
		case ToolBar::ID::QUAD:
			m_state = new RectangleEdit(m_parent);
			break;
		case ToolBar::ID::LINE:
			m_state = new LineEdit(m_parent);
			break;
		default:
			break;
		}
		if(m_state != nullptr) {
			EnqueueEventHandler(m_parent, m_state);
		}
	}
}


ConvexPolygon *EditorContext::SelectPoly(wxPoint2DDouble wpos)
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
	HistoryList *hlist = mainframe->GetHistroyList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}

void EditorContext::Redo()
{
	if(m_history >= m_actions.size()) {
		return;
	}

	m_history++;
	EditAction &back = LastAction();
	ConvexPolygon *poly;

	switch(back.base.type) {
	case EditActionType_t::LINE:
		poly = &m_polys[back.base.poly];
		poly->Slice(back.line.plane);
		poly->PurgePlanes();
		poly->ResizeAABB();
		break;
	case EditActionType_t::MOVE:
		poly = &m_polys[back.base.poly];
		poly->MoveBy(back.move.delta);
		break;
	case EditActionType_t::RECT:
		m_polys.push_back(back.rect.rect);
		poly = &m_polys.back();
		break;
	}

	wxASSERT_MSG(back.base.poly == poly - &m_polys.front(),
		"Polygon indices out of order.");

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	HistoryList *hlist = mainframe->GetHistroyList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();
}

void EditorContext::ApplyAction(EditAction action)
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
	HistoryList *hlist = mainframe->GetHistroyList();
	hlist->SetItemCount(m_actions.size());
	hlist->Refresh();

	if(!m_actions.empty() && m_history) {
		EditAction &back = LastAction();
		if(back.base.type == EditActionType_t::MOVE && action.base.type == EditActionType_t::MOVE) {
			/* we don't want to spam a move action for each pixel moved */
			if(back.base.poly == action.base.poly) {
				back.move.delta += action.move.delta;
				hlist->Refresh(false);
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
}
