
#include <array>
#include <vector>
#include <wx/geometry.h>
#include "src/convexpolygon.hpp"


ConvexPolygon::ConvexPolygon(const wxRect2DDouble &rect)
{
	m_center = rect.GetCentre();
	m_aabb   = rect;

	m_points.clear();
	/* Start off with our bounding box */
	m_points.push_back(m_aabb.GetLeftTop());
	m_points.push_back(m_aabb.GetRightTop());
	m_points.push_back(m_aabb.GetRightBottom());
	m_points.push_back(m_aabb.GetLeftBottom());
}

static
wxPoint2DDouble ClosestPointOnLine(wxPoint2DDouble a,
                                   wxPoint2DDouble b, 
                                   wxPoint2DDouble pt)
{
	wxPoint2DDouble ab = b  - a;
	wxPoint2DDouble ap = pt - a;

	double t = ap.GetDotProduct(ab) / ab.GetDotProduct(ab);

	t = std::clamp(t, 0.0, 1.0);

	return a + t * ab;
}

bool ConvexPolygon::ClosestPoint(wxPoint2DDouble mpos, double threshold, wxPoint2DDouble &pt, edge_t *edge, edge_t *exclude)
{
	bool res = false;
	double min_dist = threshold;
	size_t num_points = NumPoints();
	for(size_t i = 0; i < num_points; i++) {

		edge_t e;
		e.first  = i;
		e.second = (i + 1) % num_points;

		if(exclude != nullptr) {
			if(*exclude == e) {
				continue;
			}
		}

		wxPoint2DDouble cp = GetPoint(e.first);
		wxPoint2DDouble np = GetPoint(e.second);

		wxPoint2DDouble p = ClosestPointOnLine(cp, np, mpos);
		double dist = p.GetDistance(mpos);
		if(dist < min_dist) {
			min_dist = dist;
			pt = p;
			if(edge != nullptr) {
				*edge = e;
			}
			res = true;
		}
	}

	return res;
}

void ConvexPolygon::Slice(plane_t plane)
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
}

void ConvexPolygon::ImposePlane(plane_t plane, std::vector<wxPoint2DDouble> &out)
{
	plane.Polygon(m_points, true, out);
}

bool ConvexPolygon::ContainsPoint(wxPoint2DDouble pt) const
{
	bool res = false;
	size_t num_points = NumPoints();
	for(size_t i = 0; i < num_points; i++) {
		wxPoint2DDouble cp = GetPoint(i);
		wxPoint2DDouble np = GetPoint((i + 1) % num_points);

		if(((cp.m_y >= pt.m_y && np.m_y < pt.m_y) || (cp.m_y < pt.m_y && np.m_y >= pt.m_y)) &&
			(pt.m_x < (np.m_x - cp.m_x) * (pt.m_y - cp.m_y) / (np.m_y - cp.m_y) + cp.m_x)) {
			res = !res;
		}
	}
	return res;
}
