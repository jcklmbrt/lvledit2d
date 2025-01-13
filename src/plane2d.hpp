#ifndef _PLANE2D_HPP
#define _PLANE2D_HPP

#include <vector>
#include <wx/geometry.h>

struct Plane2D
{
public:
	Plane2D() = default;
	Plane2D(const wxPoint2DDouble &start, const wxPoint2DDouble &end);
	double SignedDistance(const wxPoint2DDouble &p) const;
	bool Line(const wxPoint2DDouble &a, const wxPoint2DDouble &b, wxPoint2DDouble &out) const;
	void Flip();
	void Offset(const wxPoint2DDouble &pt);
	void Clip(const std::vector<wxPoint2DDouble> &points, std::vector<wxPoint2DDouble> &out);
private:
	/* Ax + By + C = 0 */
	double m_a, m_b, m_c;
};

#endif