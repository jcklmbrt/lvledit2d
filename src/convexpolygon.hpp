
#ifndef _CONVEXPOLYGON_HPP
#define _CONVEXPOLYGON_HPP

#include <utility>
#include <wx/dc.h>
#include <wx/geometry.h>

#include "src/plane2d.hpp"

class EditorContext;

class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect);
	void MoveBy(wxPoint2DDouble delta);
	void Slice(Plane2D plane);
	void UndoSlice(const Plane2D &plane, const wxRect2DDouble &aabb);
	void ImposePlane(Plane2D plane, std::vector<wxPoint2DDouble> &out) const;
	static bool AllPointsBehind(const Plane2D &plane, const wxPoint2DDouble points[], size_t npoints);
	bool AllPointsBehind(const Plane2D &plane) const;
	const wxRect2DDouble &GetAABB() const { return m_aabb; }
	const std::vector<Plane2D> &GetPlanes() const { return m_planes; }
	const std::vector<wxPoint2DDouble> &GetPoints() const { return m_points; }
	void ResizeAABB();
	void PurgePlanes();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const wxRect2DDouble &rect) const;
	bool Contains(const wxPoint2DDouble &pt) const;
private:
	wxRect2DDouble m_aabb;
	std::vector<Plane2D> m_planes;
	/* internal representation of polygon */
	std::vector<wxPoint2DDouble> m_points;
};

#endif
