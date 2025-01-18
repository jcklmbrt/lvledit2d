#include <cfloat>
#include <array>
#include <vector>
#include <wx/geometry.h>
#include "src/convexpolygon.hpp"
#include "src/plane2d.hpp"


ConvexPolygon::ConvexPolygon(const wxRect2DDouble &rect)
{
	m_aabb = rect;
	/* Start off with our bounding box */
	m_points.push_back(rect.GetLeftTop());
	m_points.push_back(rect.GetRightTop());
	m_points.push_back(rect.GetRightBottom());
	m_points.push_back(rect.GetLeftBottom());
}


void ConvexPolygon::MoveBy(wxPoint2DDouble delta)
{
	for(Plane2D &plane : m_planes) {
		plane.Offset(delta);
	}

	for(wxPoint2DDouble &point : m_points) {
		point += delta;
	}

	m_aabb.Offset(delta);
}


bool ConvexPolygon::AllPointsBehind(const Plane2D &plane, const wxPoint2DDouble points[], size_t npoints)
{
	for(size_t i = 0; i < npoints; i++) {
		if(plane.SignedDistance(points[i]) > 0.0) {
			return false;
		}
	}
	return true;
}


bool ConvexPolygon::AllPointsBehind(const Plane2D &plane) const
{
	return AllPointsBehind(plane, m_points.data(), m_points.size());
}


void ConvexPolygon::ResizeAABB()
{
	wxASSERT_MSG(m_points.size() >= 2,
		"Cannot construct a rect from less than two points.");

	wxPoint2DDouble mins = { DBL_MAX, DBL_MAX };
	wxPoint2DDouble maxs = { DBL_MIN, DBL_MIN };

	for(const wxPoint2DDouble pt : m_points) {
		if(pt.m_x < mins.m_x) {
			mins.m_x = pt.m_x;
		}
		if(pt.m_y < mins.m_y) {
			mins.m_y = pt.m_y;
		}
		if(pt.m_x > maxs.m_x) {
			maxs.m_x = pt.m_x;
		}
		if(pt.m_y > maxs.m_y) {
			maxs.m_y = pt.m_y;
		}
	}

	m_aabb.SetLeftTop(mins);
	m_aabb.SetRightBottom(maxs);
}


bool ConvexPolygon::Intersects(const wxRect2DDouble &rect) const 
{
	/* Broad phase */
	if(!m_aabb.Intersects(rect)) {
		return false;
	}

	std::array points = {
		rect.GetLeftTop(),
		rect.GetRightTop(),
		rect.GetRightBottom(),
		rect.GetLeftBottom()
	};

	for(const Plane2D &plane : m_planes) {
		if(AllPointsBehind(plane, points.data(), points.size())) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Contains(const wxPoint2DDouble &pt) const
{
	if(!m_aabb.Contains(pt)) {
		return false;
	}

	for(const Plane2D &plane : m_planes) {
		if(plane.SignedDistance(pt) <= 0.0) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Intersects(const ConvexPolygon &other) const
{
	if(!m_aabb.Intersects(other.GetAABB())) {
		return false;
	}

	for(const Plane2D &plane : other.GetPlanes()) {
		if(AllPointsBehind(plane)) {
			return false;
		}
	}

	for(const Plane2D &plane : m_planes) {
		if(other.AllPointsBehind(plane)) {
			return false;
		}
	}

	return true;
}


void ConvexPolygon::PurgePlanes()
{
	/* remove all planes that aren't touching any points */
	m_planes.erase(std::remove_if(m_planes.begin(), m_planes.end(), [this](Plane2D plane) {
		constexpr double EPSILON = 0.001;
		for(const wxPoint2DDouble &pt : m_points) {
			if(plane.SignedDistance(pt) < EPSILON) {
				return false;
			}
		}
		return true;
	}), m_planes.end());
}


void ConvexPolygon::Slice(Plane2D plane)
{
	m_planes.push_back(plane);

	std::vector<wxPoint2DDouble> new_points;
	ImposePlane(plane, new_points);

	m_points = new_points;
}


void ConvexPolygon::ImposePlane(Plane2D plane, std::vector<wxPoint2DDouble> &out) const
{
	plane.Clip(m_points, out);
}
