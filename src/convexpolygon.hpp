
#ifndef _CONVEXPOLYGON_HPP
#define _CONVEXPOLYGON_HPP

#include <wx/geometry.h>

class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect);
	bool ContainsPoint(wxPoint2DDouble pt) const;
	void MoveBy(wxPoint2DDouble delta);
	wxPoint2DDouble GetCenter() const;
	size_t NumPoints() const;
	wxPoint2DDouble GetPoint(size_t i) const;
	void SetPoint(size_t i, wxPoint2DDouble pt);
private:
	wxPoint2DDouble           m_center;
	std::vector<wxPoint2DDouble> m_points;
};

inline void ConvexPolygon::MoveBy(wxPoint2DDouble delta)
{
	m_center += delta;
}

inline wxPoint2DDouble ConvexPolygon::GetCenter() const
{
	return m_center;
}

inline size_t ConvexPolygon::NumPoints() const
{
	return m_points.size();
}

inline wxPoint2DDouble ConvexPolygon::GetPoint(size_t i) const
{
	return m_points[i] + m_center;
}

inline void ConvexPolygon::SetPoint(size_t i, wxPoint2DDouble pt)
{
	m_points[i] = m_center - pt;
}

#endif