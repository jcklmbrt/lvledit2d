#include <array>
#include <algorithm>
#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/geometry.hpp"


Plane2D::Plane2D(const Point2D &start, const Point2D &end)
{
	Point2D delta = start - end;
	delta = normalize(delta);
	m_a = -delta.y;
	m_b = +delta.x;
	m_c = -(m_a * start.x + m_b * start.y);
}


void Plane2D::Flip()
{
	m_a = -m_a;
	m_b = -m_b;
	m_c = -m_c;
}


void Plane2D::Offset(const Point2D &pt)
{
	m_c -= m_a * pt.x + m_b * pt.y;
}


void Plane2D::Clip(const std::vector<Point2D> &points, std::vector<Point2D> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		Point2D a = points[i];
		Point2D b = points[(i + 1) % npoints];

		float da = SignedDistance(a);
		float db = SignedDistance(b);

		/* only "forward" side */
		if(da >= 0) {
			out.push_back(a);
		}

		if(da * db < 0) {
			Point2D isect;
			if(Line(a, b, isect)) {
				out.push_back(isect);
			}
		}
	}
}


bool Plane2D::Line(const Point2D &a, const Point2D &b, Point2D &out) const
{
	Point2D dir = b - a;

	float denom = m_a * dir.x + m_b * dir.y;

	if(denom == 0) {
		return false;
	}

	float t = -(m_a * a.x + m_b * a.y + m_c) / denom;

	out = a + t * dir;
	return true;
}


float Plane2D::SignedDistance(const Point2D &p) const
{
	return m_a * p.x + m_b * p.y + m_c;
}


bool Plane2D::operator==(const Plane2D &other) const
{
	return other.m_a == m_a && other.m_b == m_b && other.m_c == m_c;
}


ConvexPolygon::ConvexPolygon(const Rect2D &rect)
{
	m_aabb = rect;
	/* Start off with our bounding box */
	m_points.push_back(rect.GetLeftTop());
	m_points.push_back(rect.GetRightTop());
	m_points.push_back(rect.GetRightBottom());
	m_points.push_back(rect.GetLeftBottom());
}


void ConvexPolygon::MoveBy(Point2D delta)
{
	for(Plane2D &plane : m_planes) {
		plane.Offset(delta);
	}

	for(Point2D &point : m_points) {
		point += delta;
	}

	m_aabb.Offset(delta);
}


bool ConvexPolygon::AllPointsBehind(const Plane2D &plane, const Point2D points[], size_t npoints)
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


void Rect2D::FitPoints(const Point2D pts[], size_t npts)
{
	wxASSERT_MSG(npts >= 2, "Cannot construct a rect from less than two points.");

	Point2D mins = { DBL_MAX, DBL_MAX };
	Point2D maxs = { DBL_MIN, DBL_MIN };

	for(size_t i = 0; i < npts; i++) {
		Point2D pt = pts[i];
		if(pt.x < mins.x) {
			mins.x = pt.x;
		}
		if(pt.y < mins.y) {
			mins.y = pt.y;
		}
		if(pt.x > maxs.x) {
			maxs.x = pt.x;
		}
		if(pt.y > maxs.y) {
			maxs.y = pt.y;
		}
	}
	SetMins(mins);
	SetMaxs(maxs);
}


void ConvexPolygon::ResizeAABB()
{
	m_aabb.FitPoints(m_points.data(), m_points.size());
}


bool ConvexPolygon::Intersects(const Rect2D &rect) const 
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


bool ConvexPolygon::Contains(const Point2D &pt) const
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
		constexpr float EPSILON = 0.001f;
		for(const Point2D &pt : m_points) {
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

	std::vector<Point2D> new_points;
	ImposePlane(plane, new_points);

	m_points = new_points;
}


void ConvexPolygon::ImposePlane(Plane2D plane, std::vector<Point2D> &out) const
{
	plane.Clip(m_points, out);
}


GLTexture *ConvexPolygon::GetTexture() const 
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) { 
		return nullptr;
	}
	if(m_texture == -1) {
		return nullptr;
	}
	return &canvas->Editor.Textures[m_texture];
}


Rect2D ConvexPolygon::GetUV(const Rect2D &aabb) const
{
	Rect2D rect = aabb;
	if(m_texturescale != 0) {
		float scale = static_cast<float>(m_texturescale * GLBackgroundGrid::SPACING);
		Point2D mins = { 0.0f, 0.0f };
		Point2D maxs = { scale, scale };

		rect.SetMins(mins);
		rect.SetMaxs(maxs);
	}
	return rect;
}


Rect2D ConvexPolygon::GetUV() const
{
	return GetUV(m_aabb);
}
