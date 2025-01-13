#include "src/plane2d.hpp"


Plane2D::Plane2D(const wxPoint2DDouble &start, const wxPoint2DDouble &end)
{
	wxPoint2DDouble delta = start - end;
	delta.Normalize();
	m_a = -delta.m_y;
	m_b = +delta.m_x;
	m_c = -(m_a * start.m_x + m_b * start.m_y);
}


void Plane2D::Flip()
{
	m_a = -m_a;
	m_b = -m_b;
	m_c = -m_c;
}


void Plane2D::Offset(const wxPoint2DDouble &pt)
{
	m_c -= m_a * pt.m_x + m_b * pt.m_y;
}


void Plane2D::Clip(const std::vector<wxPoint2DDouble> &points, std::vector<wxPoint2DDouble> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble a = points[i];
		wxPoint2DDouble b = points[(i + 1) % npoints];

		double da = SignedDistance(a);
		double db = SignedDistance(b);

		/* only "forward" side */
		if(da >= 0) {
			out.push_back(a);
		}

		if(da * db < 0) {
			wxPoint2DDouble isect;
			if(Line(a, b, isect)) {
				out.push_back(isect);
			}
		}
	}
}


bool Plane2D::Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out) const
{
	wxPoint2DDouble dir = b - a;

	double denom = m_a * dir.m_x + m_b * dir.m_y;

	if(denom == 0) {
		return false;
	}

	double t = -(m_a * a.m_x + m_b * a.m_y + m_c) / denom;

	out = a + t * dir;
	return true;
}


double Plane2D::SignedDistance(const wxPoint2DDouble &p) const
{
	return m_a * p.m_x + m_b * p.m_y + m_c;
}
