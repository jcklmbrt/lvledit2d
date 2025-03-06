#include <array>
#include <numeric>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <glm/glm.hpp>
#include <limits>
#include <wx/debug.h>

#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glbackgroundgrid.hpp"
#include "src/geometry.hpp"


Plane2D::Plane2D(const glm::i32vec2 &start, const glm::i32vec2 &end)
{
	a = end.y - start.y;
	b = start.x - end.x;
	c = -(a * start.x + b * start.y);
	Normalize();
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
	Normalize();
}

void Plane2D::Normalize()
{
	int32_t g = std::gcd(std::gcd(std::abs(a), std::abs(b)), std::abs(c));

	if (g > 1) {
		a /= g;
		b /= g;
		c /= g;
	}
}


void Plane2D::Scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom)
{
	int64_t px = numer.x;
	int64_t qx = denom.x;
	int64_t py = numer.y;
	int64_t qy = denom.y;

	int64_t x0 = origin.x;
	int64_t y0 = origin.y;

	int64_t a_prime = static_cast<int64_t>(a) * py * qx;
	int64_t b_prime = static_cast<int64_t>(b) * px * qy;
	int64_t c_prime = static_cast<int64_t>(a) * x0 * (px * py - py * qx) +
	                  static_cast<int64_t>(b) * y0 * (px * py - px * qy) +
	                  px * py * static_cast<int64_t>(c);

	int64_t g = std::gcd(std::gcd(std::abs(a_prime), std::abs(b_prime)), std::abs(c_prime));

	if(g > 1) {
		a_prime /= g;
		b_prime /= g;
		c_prime /= g;
	}

	wxASSERT(a_prime < INT32_MAX && b_prime < INT32_MAX && c_prime < INT32_MAX);

	a = a_prime;
	b = b_prime;
	c = c_prime;
}


void Plane2D::Clip(const std::vector<glm::vec2> &points, std::vector<glm::vec2> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		glm::vec2 a = points[i];
		glm::vec2 b = points[(i + 1) % npoints];

		float da = SignedDistance(a);
		float db = SignedDistance(b);

		/* only "forward" side */
		if(da >= 0) {
			out.push_back(a);
		}

		if(da * db < 0) {
			glm::vec2 isect;
			isect.x = a.x * db - b.x * da;
			isect.y = a.y * db - b.y * da;
			isect /= db - da;
			out.push_back(isect);
		}
	}
}


float Plane2D::SignedDistance(const glm::vec2 &p) const
{
	return static_cast<float>(a) * p.x + static_cast<float>(b) * p.y + static_cast<float>(c);
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
	glm::vec2 lt = rect.mins;
	glm::vec2 rb = rect.maxs;
	glm::vec2 lb = { rect.mins.x, rect.maxs.y };
	glm::vec2 rt = { rect.maxs.x, rect.mins.y };

	/* Start off with our bounding box */
	points.push_back(lt);
	points.push_back(rt);
	points.push_back(rb);
	points.push_back(lb);
}

void ConvexPolygon::Scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom)
{
	for(Plane2D &plane : planes) {
		plane.Scale(origin, numer, denom);
	}

	for(glm::vec2 &point : points) {
		point = glm::vec2(origin) + ((point - glm::vec2(origin)) * glm::vec2(numer)) / glm::vec2(denom);
	}

	aabb.maxs = origin + ((aabb.maxs - origin) * numer) / denom;
	aabb.mins = origin + ((aabb.mins - origin) * numer) / denom;
}


void ConvexPolygon::Offset(const glm::i32vec2 &delta)
{
	for(Plane2D &plane : planes) {
		plane.Offset(delta);
	}

	for(glm::vec2 &point : points) {
		point += delta;
	}

	aabb.maxs += delta;
	aabb.mins += delta;
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

void Rect2D::FitPoints(const glm::vec2 pts[], size_t npts)
{
	wxASSERT_MSG(npts >= 2,
		"Cannot construct a rect from less than two points.");

	mins = floor(pts[0]);
	maxs = ceil(pts[0]);

	for(size_t i = 1; i < npts; i++) {
		glm::i32vec2 f = floor(pts[i]);
		glm::i32vec2 c = ceil(pts[i]);
		mins = min(f, mins);
		maxs = max(c, maxs);
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
		glm::vec2(rect.mins.x, rect.mins.y),
		glm::vec2(rect.maxs.x, rect.mins.y),
		glm::vec2(rect.mins.x, rect.maxs.y),
		glm::vec2(rect.maxs.x, rect.maxs.y)
	};

	for(const Plane2D &plane : planes) {
		if(AllPointsBehind(plane, points.data(), points.size())) {
			return false;
		}
	}

	return true;
}


bool ConvexPolygon::Contains(const glm::vec2 &pt) const
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
		for(const glm::vec2 &pt : points) {
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

	std::vector<glm::vec2> new_points;
	ImposePlane(plane, new_points);

	points = new_points;
}


void ConvexPolygon::ImposePlane(Plane2D plane, std::vector<glm::vec2> &out) const
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
		glm::vec2 mins = { 0.0f, 0.0f };
		glm::vec2 maxs = { scale, scale };

		rect.mins = { 0.0f, 0.0f };
		rect.maxs = { scale, scale };
	}
	return rect;
}


Rect2D ConvexPolygon::GetUV() const
{
	return GetUV(aabb);
}
