#include <array>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <glm/glm.hpp>
#include <limits>
#include <wx/debug.h>

#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/geometry.hpp"


Plane2D::Plane2D(const Point2D &start, const Point2D &end)
{
	Point2D delta = start - end;
	delta = normalize(delta);
	a = -delta.y;
	b = +delta.x;
	c = -(a * start.x + b * start.y);
}


void Plane2D::Flip()
{
	a = -a;
	b = -b;
	c = -c;
}


void Plane2D::Offset(const Point2D &pt)
{
	c -= a * pt.x + b * pt.y;
}


void Plane2D::Transform(const Matrix3 &t)
{
	glm::vec3 r = glm::vec3(a, b, c) * inverse(t);
	a = r.x;
	b = r.y;
	c = r.z;
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


bool Plane2D::Line(const Point2D &p0, const Point2D &p1, Point2D &out) const
{
	Point2D dir = p1 - p0;

	float denom = a * dir.x + b * dir.y;

	if(denom == 0) {
		return false;
	}

	float t = -(a * p0.x + b * p0.y + c) / denom;

	out = p0 + t * dir;
	return true;
}


float Plane2D::SignedDistance(const Point2D &p) const
{
	return a * p.x + b * p.y + c;
}


bool Plane2D::operator==(const Plane2D &other) const
{
	return other.a == a && other.b == b && other.c == c;
}


ConvexPolygon::ConvexPolygon(const Rect2D &rect)
{
	aabb = rect;
	/* Start off with our bounding box */
	points.push_back(rect.GetLeftTop());
	points.push_back(rect.GetRightTop());
	points.push_back(rect.GetRightBottom());
	points.push_back(rect.GetLeftBottom());
}


void ConvexPolygon::Transform(const Matrix3 &t)
{
	for(Plane2D &plane : planes) {
		plane.Transform(t);
	}

	for(Point2D &point : points) {
		point = t * glm::vec3(point, 1.0f);
	}

	ResizeAABB();
}

void ConvexPolygon::Scale(const Point2D &origin, const Point2D &scale)
{
	Matrix3 t = { scale.x, 0.0f, 0.0f,
		      0.0f, scale.y, 0.0f,
		        origin.x * (1.0f - scale.x),
		        origin.y * (1.0f - scale.y), 1.0f
	};

	Transform(t);
}


void ConvexPolygon::Rotate(const Point2D &origin, float angle)
{
	float cr = cos(angle);
	float sr = sin(angle);

	Matrix3 t = {  cr, sr, 0.0f,
		       -sr, cr, 0.0f,
		         origin.x * (1.0f - cr) + origin.y * sr,
			 -origin.x * sr + origin.y * (1.0f - cr), 1.0f
	};

	Transform(t);
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
	return AllPointsBehind(plane, points.data(), points.size());
}


void Rect2D::FitPoints(const Point2D pts[], size_t npts)
{
	wxASSERT_MSG(npts >= 2,
		     "Cannot construct a rect from less than two points.");

	mins = pts[0];
	maxs = pts[0];

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

	wxASSERT_MSG(mins.x < maxs.x && mins.y < maxs.y,
		     "Cannot construct a rect from equal points.");
}


void ConvexPolygon::ResizeAABB()
{
	aabb.FitPoints(points.data(), points.size());
}


bool ConvexPolygon::Intersects(const Rect2D &rect) const 
{
	/* Broad phase */
	if(!aabb.Intersects(rect)) {
		return false;
	}

	std::array points = {
		rect.GetLeftTop(),
		rect.GetRightTop(),
		rect.GetRightBottom(),
		rect.GetLeftBottom()
	};

	for(const Plane2D &plane : planes) {
		if(AllPointsBehind(plane, points.data(), points.size())) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Contains(const Point2D &pt) const
{
	if(!aabb.Contains(pt)) {
		return false;
	}

	for(const Plane2D &plane : planes) {
		if(plane.SignedDistance(pt) <= 0.0) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Intersects(const ConvexPolygon &other) const
{
	if(!aabb.Intersects(other.aabb)) {
		return false;
	}

	for(const Plane2D &plane : other.planes) {
		if(AllPointsBehind(plane)) {
			return false;
		}
	}

	for(const Plane2D &plane : planes) {
		if(other.AllPointsBehind(plane)) {
			return false;
		}
	}

	return true;
}


void ConvexPolygon::PurgePlanes()
{
	/* remove all planes that aren't touching any points */
	planes.erase(std::remove_if(planes.begin(), planes.end(), [this](Plane2D plane) {
		constexpr float EPSILON = 0.001f;
		for(const Point2D &pt : points) {
			if(plane.SignedDistance(pt) < EPSILON) {
				return false;
			}
		}
		return true;
	}), planes.end());
}


void ConvexPolygon::Slice(Plane2D plane)
{
	planes.push_back(plane);

	std::vector<Point2D> new_points;
	ImposePlane(plane, new_points);

	points = new_points;
}


void ConvexPolygon::ImposePlane(Plane2D plane, std::vector<Point2D> &out) const
{
	plane.Clip(points, out);
}


GLTexture *ConvexPolygon::GetTexture() const 
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas == nullptr) { 
		return nullptr;
	}
	if(texindex == -1) {
		return nullptr;
	}
	return &canvas->editor.textures[texindex];
}


Rect2D ConvexPolygon::GetUV(const Rect2D &aabb) const
{
	Rect2D rect = aabb;
	if(texscale != 0) {
		float scale = static_cast<float>(texscale * GLBackgroundGrid::SPACING);
		Point2D mins = { 0.0f, 0.0f };
		Point2D maxs = { scale, scale };

		rect.mins = { 0.0f, 0.0f };
		rect.maxs = { scale, scale };
	}
	return rect;
}


Rect2D ConvexPolygon::GetUV() const
{
	return GetUV(aabb);
}
