
#ifndef _CONVEXPOLYGON_HPP
#define _CONVEXPOLYGON_HPP

#include <utility>
#include <wx/dc.h>
#include <wx/geometry.h>

struct plane_t
{
public:
	plane_t() = default;
	plane_t(const wxPoint2DDouble &start, const wxPoint2DDouble &end);
private:
	/* Ax + By + C = 0 */
	double A, B, C;
};

inline plane_t::plane_t(const wxPoint2DDouble &start,
                 const wxPoint2DDouble &end)
{
	wxPoint2DDouble delta = start - end;
	delta.Normalize();
	A = -delta.m_y;
	B = +delta.m_x;
	C = -(A * start.m_x + B * start.m_y);
}


struct edge_t : public std::pair<size_t, size_t>
{

};

class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect);
	bool ContainsPoint(wxPoint2DDouble pt) const;
	bool ClosestPoint(wxPoint2DDouble mpos, double threshold, wxPoint2DDouble &pt, edge_t *edge = nullptr, edge_t *exclude = nullptr);
	void MoveBy(wxPoint2DDouble delta);
	bool Slice(plane_t plane);
	bool ImposePlane(plane_t plane);
	wxRect2DDouble GetAABB() const;
	wxPoint2DDouble GetCenter() const;
	size_t NumPoints() const;
	wxPoint2DDouble GetPoint(size_t i) const;
private:
	std::vector<plane_t>         m_planes;
	wxRect2DDouble               m_aabb;
	wxPoint2DDouble              m_center;
	std::vector<wxPoint2DDouble> m_points;
};


inline wxRect2DDouble ConvexPolygon::GetAABB() const
{
	return m_aabb;
}


inline void ConvexPolygon::MoveBy(wxPoint2DDouble delta)
{
	for(wxPoint2DDouble &p : m_points) {
		p += delta;
	}

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
	return m_points[i];
}

#endif