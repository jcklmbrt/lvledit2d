#include <array>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <glm/glm.hpp>
#include <limits>
#include <wx/debug.h>

#include "glm/matrix.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/geometry.hpp"


Plane2D::Plane2D(const glm::i32vec2 &start, const glm::i32vec2 &end)
{
	a = end.y - start.y;
	b = start.x - end.x;
	c = -(a * start.x + b * start.y);
}


void Plane2D::Flip()
{
	a = -a;
	b = -b;
	c = -c;
}


void Plane2D::Offset(const glm::i32vec2 &pt)
{
	c -= a * pt.x + b * pt.y;
}


void Plane2D::Transform(const glm::mat3 &t)
{
	glm::mat3 inv = glm::inverse(t);
	glm::vec3 r = glm::vec3(a, b, c) * inv;
	a = r.x;
	b = r.y;
	c = r.z;
}

void Plane2D::Clip(const std::vector<glm_vec2> &points, std::vector<glm_vec2> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		glm::vec2 a = points[i];
		glm::vec2 b = points[(i + 1) % npoints];

		int32_t da = SignedDistance(a);
		int32_t db = SignedDistance(b);

		/* only "forward" side */
		if(da >= 0) {
			out.push_back(a);
		}

		if(da * db < 0) {
			glm::vec2 isect;
			if(Line(a, b, isect)) {
				out.push_back(isect);
			}
		}
	}
}


bool Plane2D::Line(const glm::vec2 &p0, const glm::vec2 &p1, glm::vec2 &out) const
{
	glm::vec2 dir = p1 - p0;

	float denom = a * dir.x + b * dir.y;

	if(denom == 0) {
		return false;
	}

	float t = -(a * p0.x + b * p0.y + c) / denom;

	out = p0 + t * dir;

	return true;
}


int32_t Plane2D::SignedDistance(const glm::i32vec2 &p) const
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


void ConvexPolygon::Transform(const glm::mat3 &t)
{
	for(Plane2D &plane : planes) {
		plane.Transform(t);
	}

	for(glm_vec2 &point : points) {
		point = t * glm::i32vec3(point, 1.0f);
	}

	ResizeAABB();
}


bool ConvexPolygon::AllPointsBehind(const Plane2D &plane, const glm::vec2 points[], size_t npoints)
{
	for(size_t i = 0; i < npoints; i++) {
		if(plane.SignedDistance(points[i]) > 0.0) {
			return false;
		}
	}
	return true;
}

bool ConvexPolygon::AllPointsBehind(const Plane2D &plane, const glm::i32vec2 points[], size_t npoints)
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


void Rect2D::FitPoints(const glm_vec2 pts[], size_t npts)
{
	wxASSERT_MSG(npts >= 2,
		"Cannot construct a rect from less than two points.");

	mins = pts[0];
	maxs = pts[0];

	for(size_t i = 1; i < npts; i++) {
		glm::i32vec2 pt = pts[i];
		mins = glm::min(pt, mins);
		maxs = glm::max(pt, maxs);
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


bool ConvexPolygon::Contains(const glm_vec2 &pt) const
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
		for(const glm_vec2 &pt : points) {
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

	std::vector<glm_vec2> new_points;
	ImposePlane(plane, new_points);

	points = new_points;
}


void ConvexPolygon::ImposePlane(Plane2D plane, std::vector<glm_vec2> &out) const
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
		glm_vec2 mins = { 0.0f, 0.0f };
		glm_vec2 maxs = { scale, scale };

		rect.mins = { 0.0f, 0.0f };
		rect.maxs = { scale, scale };
	}
	return rect;
}


Rect2D ConvexPolygon::GetUV() const
{
	return GetUV(aabb);
}
