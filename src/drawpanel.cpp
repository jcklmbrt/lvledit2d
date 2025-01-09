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

#include "src/edit/editorcontext.hpp"
#include "src/edit/rectangleedit.hpp"
#include "src/edit/selectionedit.hpp"
#include "src/edit/lineedit.hpp"

DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent),
	  m_grid(this),
	  m_view(this),
	  m_editor(this)
{
	Bind(wxEVT_KEY_DOWN, &DrawPanel::OnKeyDown, this);
	Bind(wxEVT_LEFT_DOWN, &DrawPanel::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &DrawPanel::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &DrawPanel::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &DrawPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &DrawPanel::OnMouse, this);

	MainFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	PushEventHandler(&m_editor);
	PushEventHandler(&m_view);
	PushEventHandler(&m_grid);

	m_editor.OnToolSelect(static_cast<ToolBar::ID>(tool->GetId()));
}

DrawPanel::~DrawPanel()
{
	RemoveEventHandler(&m_editor);
	RemoveEventHandler(&m_view);
	RemoveEventHandler(&m_grid);
	m_editor.FinishEdit();
}


BackgroundGrid::BackgroundGrid(DrawPanel *parent)
	: m_parent(parent)
{

	Bind(wxEVT_PAINT, &BackgroundGrid::OnPaint, this);
}


void BackgroundGrid::Snap(wxPoint2DDouble &pt)
{
	pt.m_x = round(pt.m_x / SPACING) * SPACING;
	pt.m_y = round(pt.m_y / SPACING) * SPACING;
}


void BackgroundGrid::OnPaint(wxPaintEvent &e)
{
	e.Skip();
	wxPaintDC dc(m_parent);

	/* TODO: find a better color */
	dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxSize  size   = m_parent->GetSize();
	wxPoint s_mins = { 0, 0 };
	wxPoint s_maxs = { size.x, size.y };

	wxPoint2DDouble mins, maxs;
	mins = m_parent->ScreenToWorld(s_mins);
	maxs = m_parent->ScreenToWorld(s_maxs);

	mins.m_x -= SPACING + std::remainder(mins.m_x, (double)SPACING);
	mins.m_y -= SPACING + std::remainder(mins.m_y, (double)SPACING);

	for(double x = mins.m_x; x < maxs.m_x; x += SPACING) {
		wxPoint wa = m_parent->WorldToScreen({ x, mins.m_y });
		wxPoint wb = m_parent->WorldToScreen({ x, maxs.m_y });
		dc.DrawLine(wa, wb);
	}

	for(double y = mins.m_y; y < maxs.m_y; y += SPACING) {
		wxPoint wa = m_parent->WorldToScreen({ mins.m_x, y });
		wxPoint wb = m_parent->WorldToScreen({ maxs.m_x, y });
		dc.DrawLine(wa, wb);
	}
}

void DrawPanel::DrawPoint(wxPaintDC &dc, wxPoint point, const wxColor *color)
{
	wxBrush brushes[] = {
		*wxBLACK_BRUSH,
		wxBrush(*color),
		*wxBLACK_BRUSH
	};

	int sizes[] = {
		6, 4, 2
	};

	dc.SetPen(*wxTRANSPARENT_PEN);

	// highlight corners
	for(int i = 0; i < 3; i++) {
		dc.SetBrush(brushes[i]);
		int size = sizes[i];
		int size_2 = size / 2;

		wxRect cr;
		cr.SetWidth(size);
		cr.SetHeight(size);
		cr.SetX(point.x - size_2);
		cr.SetY(point.y - size_2);
		dc.DrawRectangle(cr);
	}
}


void DrawPanel::OnMouse(wxMouseEvent &e)
{
	m_mousepos = MouseToWorld(e);
	Refresh(false);
}


void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}
