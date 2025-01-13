
#ifndef _CONVEXPOLYGON_HPP
#define _CONVEXPOLYGON_HPP

#include <utility>
#include <wx/dc.h>
#include <wx/geometry.h>

#include "src/plane2d.hpp"

class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect);
	void MoveBy(wxPoint2DDouble delta);
	void Slice(Plane2D plane);
	void ImposePlane(Plane2D plane, std::vector<wxPoint2DDouble> &out) const;
	bool PointsInside(const Plane2D &plane) const;
	wxRect2DDouble GetAABB() const { return m_aabb; }
	wxPoint2DDouble GetCenter() const { return m_center; }
	const std::vector<Plane2D> &GetPlanes() const { return m_planes; }
	const std::vector<wxPoint2DDouble> &GetPoints() const { return m_points; }
	void SetupAABB();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const wxRect2DDouble &rect) const;
	bool Contains(const wxPoint2DDouble &pt) const;
private:
	std::vector<Plane2D> m_planes;
	wxRect2DDouble m_aabb;
	wxPoint2DDouble m_center;
	std::vector<wxPoint2DDouble> m_points;
};

#endif
