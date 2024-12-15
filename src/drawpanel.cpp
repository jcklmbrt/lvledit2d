
#include <cmath>
#include <wx/debug.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include <wx/pen.h>

#include "src/lvledit2d.hpp"
#include "src/toolbar.hpp"
#include "src/drawpanel.hpp"
#include "src/viewmatrix.hpp"

#include "src/edit/ibaseedit.hpp"
#include "src/edit/rectangle.hpp"
#include "src/edit/selection.hpp"

DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent)
{
	Bind(wxEVT_PAINT,    &DrawPanel::OnPaint, this, wxID_ANY);
	Bind(wxEVT_KEY_DOWN, &DrawPanel::OnKeyDown, this, wxID_ANY);

	Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnMouseLeftDown, this, wxID_ANY);
	Bind(wxEVT_LEFT_UP, &DrawPanel::OnMouseLeftUp, this, wxID_ANY);

	Bind(wxEVT_MOTION, &DrawPanel::OnMouseMotion, this, wxID_ANY);

	/* Zoom/Pan ctrl */
	Bind(wxEVT_MIDDLE_DOWN, &DrawPanel::OnMouseMiddleDown, this, wxID_ANY);
	Bind(wxEVT_MIDDLE_UP,   &DrawPanel::OnMouseMiddleUp, this, wxID_ANY);
	Bind(wxEVT_MOUSEWHEEL,  &DrawPanel::OnMouseWheel, this, wxID_ANY);
}

void DrawPanel::FinishEdit()
{
	if(m_edit != nullptr) {
		delete m_edit;
	}

	m_edit = nullptr;
}

static bool LineLine(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double a = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
	double b = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));

	if(a > 0 && a < 1 && b > 0 && b < 1) {
		return true;
	} else {
		return false;
	}
}

static bool LineRect(double x0, double y0, double x1, double y1, wxRect2DDouble rect)
{
	wxPoint2DDouble lt = rect.GetLeftTop();
	wxPoint2DDouble rt = rect.GetRightTop();
	wxPoint2DDouble lb = rect.GetLeftBottom();
	wxPoint2DDouble rb = rect.GetRightBottom();

	return  LineLine(lt.m_x, lt.m_y, rt.m_x, rt.m_y, x0, y0, x1, y1) || /* top */
		LineLine(lb.m_x, lb.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1) || /* bottom */
		LineLine(lt.m_x, lt.m_y, lb.m_x, lb.m_y, x0, y0, x1, y1) || /* left */
		LineLine(rt.m_x, rt.m_y, rb.m_x, rb.m_y, x0, y0, x1, y1);   /* right */
}

static wxRect RectAroundPoint(double x, double y, double size)
{
	wxRect r;
	double size_2 = size / 2.0;
	r.x = x - size_2;
	r.y = y - size_2;
	r.width  = size;
	r.height = size;
	return r;
}


static void DrawGrid(wxPaintDC &dc, ViewMatrix &view, int spacing, wxSize size)
{
	/* TODO: find a better color */
	dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxPoint s_mins = { 0, 0 };
	wxPoint s_maxs = { size.x, size.y };

	wxPoint2DDouble mins, maxs;
	mins = view.ScreenToWorld(s_mins);
	maxs = view.ScreenToWorld(s_maxs);

	mins.m_x -= spacing + std::remainder(mins.m_x, (double)spacing);
	mins.m_y -= spacing + std::remainder(mins.m_y, (double)spacing);

	for(double x = mins.m_x; x < maxs.m_x; x += spacing) {
		wxPoint wa = view.WorldToScreen({ x, mins.m_y });
		wxPoint wb = view.WorldToScreen({ x, maxs.m_y });
		dc.DrawLine(wa, wb);
	}

	for(double y = mins.m_y; y < maxs.m_y; y += spacing) {
		wxPoint wa = view.WorldToScreen({ mins.m_x, y });
		wxPoint wb = view.WorldToScreen({ maxs.m_x, y });
		dc.DrawLine(wa, wb);
	}
}


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);

	SetupMatrix();

	DrawGrid(dc, *dynamic_cast<ViewMatrix *>(this), m_gridspacing, GetSize());

	std::vector<wxPoint> s_points;
	
	for(const ConvexPolygon &p : *this) {
		
		dc.SetBrush(*wxTRANSPARENT_BRUSH);

		for(int i = 0; i < p.NumPoints(); i++) {
			wxPoint s_point = WorldToScreen(p.GetPoint(i));
			s_points.push_back(s_point);
		}

		dc.SetPen(wxPen(*wxBLACK, 3));
		dc.DrawPolygon(s_points.size(), s_points.data());

		if(p.ContainsPoint(m_mousepos)) {
			dc.SetPen(wxPen(*wxGREEN, 1));
		} else {
			dc.SetPen(wxPen(*wxWHITE, 1));
		}

		dc.DrawPolygon(s_points.size(), s_points.data());

		for(wxPoint s_point : s_points) {
			dc.SetPen(*wxTRANSPARENT_PEN);

			wxBrush brushes[] = {
				*wxBLACK_BRUSH,
				wxBrush(*wxWHITE),
				*wxBLACK_BRUSH
			};

			int sizes[] = {
				6, 4, 2
			};

			// highlight corners
			for(int i = 0; i < 3; i++) {
				dc.SetBrush(brushes[i]);
				int size = sizes[i];
				wxRect cr = RectAroundPoint(s_point.x, s_point.y, size);
				dc.DrawRectangle(cr);
			}
		}

		s_points.clear();
	}

	if(m_edit != nullptr) {
		m_edit->OnPaint(dc);
	}
}

void DrawPanel::OnMouseLeftDown(wxMouseEvent &e)
{
	if(m_edit == nullptr) {
		MainFrame *mainframe = wxGetApp().GetMainFrame();
		ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
		wxToolBarToolBase *tool = toolbar->GetSelected();

		switch(tool->GetId()) {
		case ToolBar::ID::SELECT:
			m_edit = new SelectionEdit(this);
			break;
		case ToolBar::ID::QUAD:
			m_edit = new RectangleEdit(this);
			break;
		default:
			FinishEdit();
			break;
		}
	}
		
	if(m_edit != nullptr) {
		m_edit->OnMouseLeftDown(e);
	}
}

void DrawPanel::OnMouseLeftUp(wxMouseEvent &e)
{
	if(m_edit != nullptr) {
		m_edit->OnMouseLeftUp(e);
	}
}

void DrawPanel::OnMouseMotion(wxMouseEvent &e)
{
	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	wxPoint2DDouble wpos = ScreenToWorld(wxpos);
	m_mousepos = wpos;

	if(m_edit != nullptr) {
		m_edit->OnMouseMotion(e);
	}

	if(m_inpan && !e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		m_inpan = false;
	}

	if(m_inpan) {
		Pan(pos - m_lastmousepos);
		m_lastmousepos = pos;
	}

	Refresh(false);
}

void DrawPanel::OnMouseWheel(wxMouseEvent &e)
{
	wxPoint wxpos = e.GetPosition();
	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	if(e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {
		int rot = e.GetWheelRotation();
		if(rot == 0) {
			/* no scroll */
		}
		else if(rot > 0) { /* scroll up */
			Zoom(pos, 1.1f);
		}
		else { /* scroll down */
			Zoom(pos, 0.9f);
		}
	}

	Refresh(false);
}

void DrawPanel::OnMouseMiddleDown(wxMouseEvent &e) 
{
	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	m_lastmousepos = pos;
	m_inpan = true;
}

void DrawPanel::OnMouseMiddleUp(wxMouseEvent &e)
{
	m_inpan = false;
}

void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}
