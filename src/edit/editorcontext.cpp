#include <array>
#include "src/drawpanel.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/lineedit.hpp"
#include "src/edit/editorcontext.hpp"


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

		/* inflate bb to illustrate planes */
		aabb.Inset(-50, -50);

		std::array aabbpts = {
			aabb.GetLeftTop(),
			aabb.GetRightTop(),
			aabb.GetRightBottom(),
			aabb.GetLeftBottom()
		};

		std::vector<wxPoint> plane_lines;

		for(const Plane &plane : p->GetPlanes()) {
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

		dc.SetPen(wxPen(*wxBLACK, 1));
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

		wxPoint2DDouble center = p->GetCenter();
		wxPoint s_center = m_panel->WorldToScreen(center);

		m_panel->DrawPoint(dc, s_center, wxYELLOW);
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