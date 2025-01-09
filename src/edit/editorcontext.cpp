#include "src/drawpanel.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"


void IBaseEdit::DrawPolygon(wxPaintDC &dc, const ConvexPolygon *p)
{
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	size_t npoints = p->NumPoints();
	wxPoint *s_points = new wxPoint[npoints];

	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble point = p->GetPoint(i);
		s_points[i] = m_panel->WorldToScreen(point);
	}

	dc.SetPen(wxPen(*wxBLACK, 3));
	dc.DrawPolygon(npoints, s_points);

	if(p == m_poly) {
		dc.SetPen(wxPen(*wxGREEN, 1));
	}
	else {
		dc.SetPen(wxPen(*wxWHITE, 1));
	}

	dc.DrawPolygon(npoints, s_points);

	for(size_t i = 0; i < npoints; i++) {
		m_panel->DrawPoint(dc, s_points[i], wxWHITE);
	}

	wxPoint2DDouble center = p->GetCenter();
	wxPoint s_center = m_panel->WorldToScreen(center);

	m_panel->DrawPoint(dc, s_center, wxYELLOW);

	delete[] s_points;
}


IBaseEdit::IBaseEdit(DrawPanel *panel)
	: m_panel(panel),
	m_poly(nullptr)
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

void EditorContext::OnPaint(wxPaintEvent &e)
{
	e.Skip();
	wxPaintDC dc(m_parent);

	for(ConvexPolygon &p : m_polys) {
		if(m_state) {
			m_state->DrawPolygon(dc, &p);
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
		FinishEdit();
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

void EditorContext::FinishEdit()
{
	if(m_state != nullptr) {
		m_parent->RemoveEventHandler(m_state);
		delete m_state;
	}

	m_state = nullptr;
}


ConvexPolygon *EditorContext::SelectPoly(wxPoint2DDouble wpos)
{
	for(ConvexPolygon &p : m_polys) {
		if(p.ContainsPoint(wpos)) {
			return &p;
		}
	}

	return nullptr;
}


ConvexPolygon *EditorContext::ClosestPoly(wxPoint2DDouble wpos, double threshold)
{
	ConvexPolygon *poly = SelectPoly(wpos);

	/* we are already inside */
	if(poly != nullptr) {
		return poly;
	}

	double min_dist = threshold;
	size_t num_polys = m_polys.size();
	for(size_t i = 0; i < num_polys; i++) {
		ConvexPolygon &p = m_polys.at(i);
		wxPoint2DDouble d = p.GetCenter() - wpos;
		double dist = d.GetVectorLength();
		if(dist < min_dist) {
			min_dist = dist;
			poly = &p;
		}
	}

	return poly;
}