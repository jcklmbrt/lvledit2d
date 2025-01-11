
#include <array>
#include <vector>
#include <wx/geometry.h>
#include "src/convexpolygon.hpp"


ConvexPolygon::ConvexPolygon(const wxRect2DDouble &rect)
{
	m_center = rect.GetCentre();
	m_aabb   = rect;

	/* Start off with our bounding box */
	m_points.push_back(m_aabb.GetLeftTop());
	m_points.push_back(m_aabb.GetRightTop());
	m_points.push_back(m_aabb.GetRightBottom());
	m_points.push_back(m_aabb.GetLeftBottom());
}


static bool PointsInsidePlane(const Plane &plane, const wxPoint2DDouble points[], size_t npoints)
{
	for(size_t i = 0; i < npoints; i++) {
		if(plane.SignedDistance(points[i]) > 0.0) {
			return false;
		}
	}
	return true;
}


bool ConvexPolygon::PointsInside(const Plane &plane) const
{
	return PointsInsidePlane(plane, m_points.data(), m_points.size());
}


void ConvexPolygon::SetupAABB()
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

	for(const Plane &plane : m_planes) {
		if(PointsInsidePlane(plane, points.data(), points.size())) {
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

	for(const Plane &plane : m_planes) {
		if(plane.SignedDistance(pt) <= 0.0) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Intersects(const ConvexPolygon &other) const
{
	/* Broad phase */
	if(!m_aabb.Intersects(other.GetAABB())) {
		return false;
	}

	for(const Plane &plane : other.GetPlanes()) {
		if(PointsInside(plane)) {
			return false;
		}
	}

	for(const Plane &plane : m_planes) {
		if(other.PointsInside(plane)) {
			return false;
		}
	}

	return true;
}


void ConvexPolygon::Slice(Plane plane)
{
	m_planes.push_back(plane);

	std::vector<wxPoint2DDouble> new_points;
	ImposePlane(plane, new_points);

	wxPoint2DDouble sum = { 0.0, 0.0 };
	for(wxPoint2DDouble p : new_points) {
		sum += p;
	}

	m_center = sum / static_cast<double>(new_points.size());
	m_points = new_points;

	SetupAABB();
}


void ConvexPolygon::ImposePlane(Plane plane, std::vector<wxPoint2DDouble> &out) const
{
	plane.Clip(m_points, out);
}
