
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
	double SignedDistance(const wxPoint2DDouble &p);
	bool Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out);
	void Flip();
	void Polygon(const std::vector<wxPoint2DDouble> &points, bool sign, std::vector<wxPoint2DDouble> &out);
private:
	/* Ax + By + C = 0 */
	double A, B, C;
};

inline void plane_t::Flip()
{
	A = -A;
	B = -B;
	C = -C;
}

inline void plane_t::Polygon(const std::vector<wxPoint2DDouble> &points, bool sign, std::vector<wxPoint2DDouble> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		wxPoint2DDouble a = points[i];
		wxPoint2DDouble b = points[(i + 1) % npoints];

		double da = SignedDistance(a);
		double db = SignedDistance(b);

		if(sign && da >= 0) {
			out.push_back(a);
		}
		if(!sign && da <= 0) {
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


inline bool plane_t::Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out)
{
	wxPoint2DDouble dir = b - a;

	double denom = A * dir.m_x + B * dir.m_y;

	if(denom == 0) {
		return false;
	}

	double t = -(A * a.m_x + B * a.m_y + C) / denom;

	out = a + t * dir;
	return true;
}

inline double plane_t::SignedDistance(const wxPoint2DDouble &p)
{
	return A * p.m_x + B * p.m_y + C;
}

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
	void Slice(plane_t plane);
	void ImposePlane(plane_t plane, std::vector<wxPoint2DDouble> &out);
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

	m_aabb.Offset(delta);
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
