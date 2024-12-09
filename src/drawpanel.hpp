
#ifndef _DRAWPANEL_HPP
#define _DRAWPANEL_HPP

#include <vector>
#include <wx/geometry.h>
#include <wx/wx.h>

#include "src/viewmatrix.hpp"

class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect)
	{
		m_center = rect.GetCentre();

		m_points.push_back(m_center - rect.GetLeftTop());
		m_points.push_back(m_center - rect.GetRightTop());
		m_points.push_back(m_center - rect.GetRightBottom());
		m_points.push_back(m_center - rect.GetLeftBottom());
	}

	inline bool ContainsPoint(wxPoint2DDouble pt) const
	{
		bool res = false;
		size_t num_points = NumPoints();
		for(size_t i = 0; i < num_points; i++) {
			wxPoint2DDouble cp = GetPoint(i);
			wxPoint2DDouble np = GetPoint((i + 1) % num_points);

			if(((cp.m_y >= pt.m_y && np.m_y  < pt.m_y) || (cp.m_y  < pt.m_y && np.m_y >= pt.m_y)) &&
			    (pt.m_x < (np.m_x - cp.m_x) * (pt.m_y - cp.m_y) / (np.m_y - cp.m_y) + cp.m_x)) {
				res = !res;
			}
		}
		return res;
	}

	inline void MoveBy(wxPoint2DDouble delta)
	{
		m_center += delta;
	}

	inline wxPoint2DDouble GetCenter() const
	{
		return m_center;
	}

	inline size_t NumPoints() const
	{
		return m_points.size();
	}

	inline wxPoint2DDouble GetPoint(size_t i) const
	{
		return m_points[i] + m_center;
	}

	inline void SetPoint(size_t i, wxPoint2DDouble pt)
	{
		m_points[i] = m_center - pt;
	}
private:
	wxPoint2DDouble           m_center;
	std::vector<wxPoint2DDouble> m_points;
};

class DrawPanel : public wxPanel
{
public:
	DrawPanel(wxWindow *parent);
	inline void FinishEdit() { m_inedit = 0; }
private:
	void OnPaint(wxPaintEvent &e);
	void OnMouse(wxMouseEvent &e);
	void OnKeyDown(wxKeyEvent &e);

	void DrawRect(wxPaintDC &dc, wxRect2DDouble rect, wxColour color, bool tmp);
	void DrawGrid(wxPaintDC &dc);

	ViewMatrix      m_view;
	wxPoint2DDouble m_mousepos;

	std::vector<ConvexPolygon> m_polys = {};

	/* editing context */
	int m_inedit = 0;

	wxRect2DDouble  m_tmprect;
	wxPoint2DDouble m_editstart;
	int m_selectedpoly;

	int  m_gridspacing = 100;
	bool m_snaptogrid  = false;

	wxDECLARE_EVENT_TABLE();
};

#endif
