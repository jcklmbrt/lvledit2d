
#include "src/convexpolygon.hpp"


ConvexPolygon::ConvexPolygon(const wxRect2DDouble &rect)
{
	m_center = rect.GetCentre();

	m_points.push_back(m_center - rect.GetLeftTop());
	m_points.push_back(m_center - rect.GetRightTop());
	m_points.push_back(m_center - rect.GetRightBottom());
	m_points.push_back(m_center - rect.GetLeftBottom());
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