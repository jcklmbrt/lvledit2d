
#ifndef _CONVEXPOLYGON_HPP
#define _CONVEXPOLYGON_HPP

#include <utility>
#include <wx/dc.h>
#include <wx/geometry.h>

struct Plane
{
public:
	Plane() = default;
	Plane(const wxPoint2DDouble &start, const wxPoint2DDouble &end);
	double SignedDistance(const wxPoint2DDouble &p) const;
	bool Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out) const;
	void Flip();
	void Offset(const wxPoint2DDouble &pt);
	void Clip(const std::vector<wxPoint2DDouble> &points, std::vector<wxPoint2DDouble> &out);
private:
	/* Ax + By + C = 0 */
	double m_a, m_b, m_c;
};

inline void Plane::Flip()
{
	m_a = -m_a;
	m_b = -m_b;
	m_c = -m_c;
}

inline void Plane::Offset(const wxPoint2DDouble &pt)
{
	m_c -= m_a * pt.m_x + m_b * pt.m_y;
}

inline void Plane::Clip(const std::vector<wxPoint2DDouble> &points, std::vector<wxPoint2DDouble> &out) {

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


inline bool Plane::Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out) const
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

inline double Plane::SignedDistance(const wxPoint2DDouble &p) const
{
	return m_a * p.m_x + m_b * p.m_y + m_c;
}

inline Plane::Plane(const wxPoint2DDouble &start,
                 const wxPoint2DDouble &end)
{
	wxPoint2DDouble delta = start - end;
	delta.Normalize();
	m_a = -delta.m_y;
	m_b = +delta.m_x;
	m_c = -(m_a * start.m_x + m_b * start.m_y);
}


class ConvexPolygon
{
public:
	ConvexPolygon(const wxRect2DDouble &rect);
	void MoveBy(wxPoint2DDouble delta);
	void Slice(Plane plane);
	void ImposePlane(Plane plane, std::vector<wxPoint2DDouble> &out) const;
	bool PointsInside(const Plane &plane) const;
	wxRect2DDouble GetAABB() const { return m_aabb; }
	wxPoint2DDouble GetCenter() const { return m_center; }
	const std::vector<Plane> &GetPlanes() const { return m_planes; }
	const std::vector<wxPoint2DDouble> &GetPoints() const { return m_points; }
	void SetupAABB();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const wxRect2DDouble &rect) const;
	bool Contains(const wxPoint2DDouble &pt) const;
private:
	std::vector<Plane> m_planes;
	wxRect2DDouble m_aabb;
	wxPoint2DDouble m_center;
	std::vector<wxPoint2DDouble> m_points;
};


inline void ConvexPolygon::MoveBy(wxPoint2DDouble delta)
{
	for(Plane &plane : m_planes) {
		plane.Offset(delta);
	}

	for(wxPoint2DDouble &point : m_points) {
		point += delta;
	}

	m_aabb.Offset(delta);
	m_center += delta;
}

#endif
