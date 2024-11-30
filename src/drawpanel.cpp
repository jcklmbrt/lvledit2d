#include <wx/gdicmn.h>
#include <wx/geometry.h>
#include <wx/wx.h>
#include <wx/pen.h>

#include "src/lvledit2d.hpp"
#include "src/toolbar.hpp"
#include "src/drawpanel.hpp"


wxBEGIN_EVENT_TABLE(DrawPanel, wxPanel)
	EVT_PAINT(DrawPanel::OnPaint)
	EVT_MOUSE_EVENTS(DrawPanel::OnMouse)
	EVT_KEY_DOWN(DrawPanel::OnKeyDown)
wxEND_EVENT_TABLE()

void DrawPanel::WorldToScreen(wxPoint2DDouble world, wxPoint &screen)
{
	world = m_view.TransformPoint(world);
	screen.x = static_cast<int>(world.m_x);
	screen.y = static_cast<int>(world.m_y);
}

void DrawPanel::ScreenToWorld(wxPoint screen, wxPoint2DDouble &world)
{
	world.m_x = static_cast<wxDouble>(screen.x);
	world.m_y = static_cast<wxDouble>(screen.y);

	wxAffineMatrix2D inv = m_view;
	inv.Invert();
	world = inv.TransformPoint(world);
}

void DrawPanel::SetupView()
{
	m_view = wxAffineMatrix2D();
	m_view.Scale(m_zoom, m_zoom);
	m_view.Translate(m_pan.m_x, m_pan.m_y);
}

void DrawPanel::Zoom(wxPoint2DDouble p, wxDouble factor)
{
	float zoom = m_zoom * factor;
	if(zoom > MAX_ZOOM || zoom < MIN_ZOOM) {
		return;
	}

	auto pan = (m_pan - p / m_zoom) + p / zoom;

	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
		return;
	}

	m_zoom = zoom;
	m_pan = pan;
}

void DrawPanel::Pan(wxPoint2DDouble delta)
{
	wxPoint2DDouble pan = m_pan + delta / m_zoom;
	if(pan.m_x > MAX_PAN_X || pan.m_x < MIN_PAN_X ||
	   pan.m_y > MAX_PAN_Y || pan.m_y < MIN_PAN_Y) {
		return;
	}
	m_pan = pan;
}

DrawPanel::DrawPanel(wxWindow *parent)
	: wxPanel(parent)
{
	wxASSERT(m_view.IsIdentity() == true);
	SetBackgroundColour(wxColour(0xFF, 0xFF, 0xEA));
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


void DrawPanel::OnPaint(wxPaintEvent &e)
{
	wxPaintDC dc(this);
	SetupView();

	dc.SetPen(wxPen(wxColour(220,220,220), 1));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	static int spacing = 100;
	for(int i = MIN_PAN_X / spacing; i < MAX_PAN_X / spacing; i++) {
		wxPoint wa,wb;
		double  space = static_cast<double>(i * spacing);
		WorldToScreen({space, MIN_PAN_Y}, wa);
		WorldToScreen({space, MAX_PAN_Y}, wb);
		dc.DrawLine(wa, wb);
		WorldToScreen({MIN_PAN_X, space}, wa);
		WorldToScreen({MAX_PAN_Y, space}, wb);
		dc.DrawLine(wa, wb);
	}

	wxPen pens[2] = { wxPen(*wxBLACK, 3), wxPen(*wxWHITE, 1) };
	for(wxPen pen : pens) {
		for(const wxRect2DDouble &r : m_rects) {
			wxPoint2DDouble lt, rb;
			lt.m_x = r.GetLeft();
			lt.m_y = r.GetTop();
			rb.m_x = r.GetRight();
			rb.m_y = r.GetBottom();

			wxPoint slt,srb;
			WorldToScreen(lt, slt);
			WorldToScreen(rb, srb);

			bool intersects = false;
			for(const wxRect2DDouble &rect : m_rects) {
				if(rect.Intersects(r) && &rect != &r) {
					intersects = true;
					break;
				}
			}
			if(intersects && pen.GetColour() == *wxWHITE) {
				dc.SetPen(wxPen(*wxRED, 1));
			} else {
				dc.SetPen(pen);
			}

			wxSize size;
			size.x = srb.x - slt.x;
			size.y = srb.y - slt.y;

			dc.DrawRectangle(slt, size);
		}
	}

	dc.SetPen(*wxTRANSPARENT_PEN);
	wxBrush brushes[] = {
		*wxBLACK_BRUSH,
		*wxRED_BRUSH,
		*wxBLACK_BRUSH
	};

	int sizes[] = {
		6,4,2
	};

	// highlight corners
	for(int i = 0; i < 3; i++) {
		dc.SetBrush(brushes[i]);
		for(wxRect2DDouble r : m_rects) {
			wxPoint2DDouble lt, rb;
			lt.m_x = r.GetLeft();
			lt.m_y = r.GetTop();
			rb.m_x = r.GetRight();
			rb.m_y = r.GetBottom();

			wxPoint slt, srb;
			WorldToScreen(lt, slt);
			WorldToScreen(rb, srb);

			int size = sizes[i];



			wxRect corners[] = {
				RectAroundPoint(srb.x, slt.y, size),
				RectAroundPoint(slt.x, srb.y, size),
				RectAroundPoint(srb.x, srb.y, size),
				RectAroundPoint(slt.x, slt.y, size),
			};

			for(wxRect corner : corners)
			{
				dc.DrawRectangle(corner);
			}
		}
	}
}

void DrawPanel::OnMouse(wxMouseEvent &e)
{
	static wxPoint2DDouble last_pos;

	wxPoint wxpos = e.GetPosition();

	wxPoint2DDouble pos;
	pos.m_x = static_cast<float>(wxpos.x);
	pos.m_y = static_cast<float>(wxpos.y);

	SetupView();
	wxPoint2DDouble world_pos;
	ScreenToWorld(wxpos, world_pos);

	wxFrame *mainframe = wxGetApp().GetMainFrame();
	ToolBar *toolbar = dynamic_cast<ToolBar *>(mainframe->GetToolBar());
	wxToolBarToolBase *tool = toolbar->GetSelected();

	static wxRect2DDouble *cur_rect = nullptr;
	static wxPoint2DDouble start;

	switch(tool->GetId())
	{
	case ToolBar::ID::QUAD:
		if(e.ButtonDown(wxMOUSE_BTN_LEFT)) {
			if(cur_rect == nullptr) {
				bool intersects = false;
				for(const wxRect2DDouble &rect : m_rects) {
					if(rect.Contains(world_pos)) {
						intersects = true;
						break;
					}
				}
				if(!intersects) {
					wxRect2DDouble r;
					r.SetCentre(world_pos);
					m_rects.push_back(r);
					start = world_pos;
					cur_rect = &m_rects.back();
				}
			}
			else {
				wxSize size = cur_rect->GetSize();
				wxASSERT(size.x >= 0 && size.y >= 0);
				bool intersects = false;
				for(const wxRect2DDouble &rect : m_rects) {
					if(rect.Intersects(*cur_rect) && &rect != cur_rect) {
						intersects = true;
						break;
					}
				}
				if(!intersects) {
					cur_rect = nullptr;
				}
			}
		}
		else if(cur_rect != nullptr) {
			double x = world_pos.m_x;
			double y = world_pos.m_y;

			wxRect2DDouble r = *cur_rect;

			if(x > start.m_x) {
				r.SetRight(x);
			} else {
				r.SetLeft(x);
			} 
			if(y > start.m_y) {
				r.SetBottom(y);
			} else {
				r.SetTop(y);
			}

			*cur_rect = r;
		}
		break;
	default:
		break;
	}

	wxString s;
	s.Printf("%s (%f,%f)", tool->GetLabel(), world_pos.m_x, world_pos.m_y);

	mainframe->SetStatusText(s);

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

	if(e.ButtonDown(wxMOUSE_BTN_MIDDLE)) {
		last_pos = pos;
	}

	if(e.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
		Pan(pos - last_pos);
		last_pos = pos;
	}

	Refresh(false);
}

void DrawPanel::OnKeyDown(wxKeyEvent &e)
{

}
