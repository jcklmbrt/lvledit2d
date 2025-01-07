
#include <array>
#include "src/convexpolygon.hpp"


ConvexPolygon::ConvexPolygon(const wxRect2DDouble &rect)
{
	m_center = rect.GetCentre();
	m_aabb   = rect;
	m_points.clear();
}



bool ConvexPolygon::Slice(plane_t plane)
{
	m_planes.push_back(plane);
	return true;
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


void ConvexPolygon::SetupPoints()
{
	m_points.clear();
	/* Start off with our bounding box */
	m_points.push_back(m_aabb.GetLeftTop());
	m_points.push_back(m_aabb.GetRightTop());
	m_points.push_back(m_aabb.GetRightBottom());
	m_points.push_back(m_aabb.GetLeftBottom());

	for(plane_t plane : m_planes) {
		
		std::vector<wxPoint2DDouble> pos;
		std::vector<wxPoint2DDouble> neg;

		size_t npoints = m_points.size();

		pos.clear();
		neg.clear();

		for(size_t i = 0; i < npoints; i++) {
			wxPoint2DDouble a = m_points[i];
			wxPoint2DDouble b = m_points[(i + 1) % npoints];

			double da = plane.SignedDistance(a);
			double db = plane.SignedDistance(b);

			if(da >= 0) {
				pos.push_back(a);
			}
			if(da <= 0) {
				neg.push_back(a);
			}

			if(da * db < 0) {
				wxPoint2DDouble isect;
				if(plane.Line(a, b, isect)) {
					pos.push_back(isect);
					neg.push_back(isect);
				}
			}
		}

		m_points = neg;

	}
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