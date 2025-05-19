#include <numeric>
#include <cassert>
#include <glm/glm.hpp>

#include "src/edit/editorcontext.hpp"
#include "src/gl/glcontext.hpp"
#include "src/geometry.hpp"


iline2d::iline2d(const glm::i32vec2 &start, const glm::i32vec2 &end)
{
	a = end.y - start.y;
	b = start.x - end.x;
	c = -(a * start.x + b * start.y);
	normalize();
}


void iline2d::flip()
{
	a = -a;
	b = -b;
	c = -c;
}


void iline2d::offset(const glm::i32vec2 &pt)
{
	c -= a * pt.x + b * pt.y;
	normalize();
}

void iline2d::normalize()
{
	int32_t g = std::gcd(std::gcd(std::abs(a), std::abs(b)), std::abs(c));

	if(g > 1) {
		a /= g;
		b /= g;
		c /= g;
	}
}


void iline2d::scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom)
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

	assert(a_prime < INT32_MAX && b_prime < INT32_MAX && c_prime < INT32_MAX);

	a = a_prime;
	b = b_prime;
	c = c_prime;
}


void iline2d::clip(const std::vector<glm::vec2> &points, std::vector<glm::vec2> &out) {

	size_t npoints = points.size();
	for(size_t i = 0; i < npoints; i++) {
		glm::vec2 a = points[i];
		glm::vec2 b = points[(i + 1) % npoints];

		float da = distnumer(a);
		float db = distnumer(b);

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


float iline2d::distnumer(const glm::vec2 &p) const
{
	return static_cast<float>(a) * p.x + static_cast<float>(b) * p.y + static_cast<float>(c);
}


int32_t iline2d::distnumer(const glm::i32vec2 &p) const
{
	return a * p.x + b * p.y + c;
}


bool iline2d::operator==(const iline2d &other) const
{
	return other.a == a && other.b == b && other.c == c;
}


poly2d::poly2d(const irect2d &rect)
{
	m_aabb = rect;
	glm::vec2 lt = rect.mins;
	glm::vec2 rb = rect.maxs;
	glm::vec2 lb = { rect.mins.x, rect.maxs.y };
	glm::vec2 rt = { rect.maxs.x, rect.mins.y };

	/* Start off with our bounding box */
	m_points.push_back(lt);
	m_points.push_back(rt);
	m_points.push_back(rb);
	m_points.push_back(lb);
}

void poly2d::scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom)
{
	for(iline2d &plane : m_lines) {
		plane.scale(origin, numer, denom);
	}

	for(glm::vec2 &point : m_points) {
		point = glm::vec2(origin) + ((point - glm::vec2(origin)) * glm::vec2(numer)) / glm::vec2(denom);
	}

	m_aabb.maxs = origin + ((m_aabb.maxs - origin) * numer) / denom;
	m_aabb.mins = origin + ((m_aabb.mins - origin) * numer) / denom;
}


void poly2d::offset(const glm::i32vec2 &delta)
{
	for(iline2d &plane : m_lines) {
		plane.offset(delta);
	}

	for(glm::vec2 &point : m_points) {
		point += delta;
	}

	m_aabb.maxs += delta;
	m_aabb.mins += delta;
}


bool poly2d::allptsbehind(const iline2d &plane, const glm::vec2 points[], size_t npoints)
{
	for(size_t i = 0; i < npoints; i++) {
		if(plane.distnumer(points[i]) > 0.0) {
			return false;
		}
	}
	return true;
}

bool poly2d::allptsbehind(const iline2d &plane, const glm::i32vec2 points[], size_t npoints)
{
	for(size_t i = 0; i < npoints; i++) {
		if(plane.distnumer(points[i]) > 0.0) {
			return false;
		}
	}
	return true;
}

bool poly2d::allptsbehind(const iline2d &plane) const
{
	return allptsbehind(plane, m_points.data(), m_points.size());
}

void irect2d::fit(const glm::vec2 pts[], size_t npts)
{
	assert(npts >= 2);

	mins = floor(pts[0]);
	maxs = ceil(pts[0]);

	for(size_t i = 1; i < npts; i++) {
		glm::i32vec2 f = floor(pts[i]);
		glm::i32vec2 c = ceil(pts[i]);
		mins = min(f, mins);
		maxs = max(c, maxs);
	}

	assert(mins.x < maxs.x && mins.y < maxs.y);
}


void poly2d::fitaabb()
{
	m_aabb.fit(m_points.data(), m_points.size());
}


bool poly2d::intersects(const irect2d &rect) const 
{
	/* Broad phase */
	if(!m_aabb.intersects(rect)) {
		return false;
	}

	std::array points = {
		glm::vec2(rect.mins.x, rect.mins.y),
		glm::vec2(rect.maxs.x, rect.mins.y),
		glm::vec2(rect.mins.x, rect.maxs.y),
		glm::vec2(rect.maxs.x, rect.maxs.y)
	};

	for(const iline2d &plane : m_lines) {
		if(allptsbehind(plane, points.data(), points.size())) {
			return false;
		}
	}

	return true;
}


bool poly2d::contains(const glm::vec2 &pt) const
{
	if(!m_aabb.contains(pt)) {
		return false;
	}

	for(const iline2d &plane : m_lines) {
		if(plane.distnumer(pt) <= 0.0) {
			return false;
		}
	}

	return true;
}


bool poly2d::intersects(const poly2d &other) const
{
	if(!m_aabb.intersects(other.m_aabb)) {
		return false;
	}

	for(const iline2d &plane : other.m_lines) {
		if(allptsbehind(plane)) {
			return false;
		}
	}

	for(const iline2d &plane : m_lines) {
		if(other.allptsbehind(plane)) {
			return false;
		}
	}

	return true;
}


void poly2d::fitlines()
{
	/* remove all planes that aren't touching any points */
	m_lines.erase(std::remove_if(m_lines.begin(), m_lines.end(), [this](iline2d plane) {
		constexpr float EPSILON = 0.001f;
		for(const glm::vec2 &pt : m_points) {
			if(plane.distnumer(pt) < EPSILON) {
				return false;
			}
		}
		return true;
	}), m_lines.end());
}


void poly2d::slice(iline2d plane)
{
	m_lines.push_back(plane);

	std::vector<glm::vec2> new_points;
	addline(plane, new_points);

	m_points = new_points;
}


void poly2d::addline(iline2d plane, std::vector<glm::vec2> &out) const
{
	plane.clip(m_points, out);
}


gl::texture *poly2d::texture() const 
{
	/*
	l2d::editor *editor = l2d::editor::current();
	if(editor == nullptr) { 
		return nullptr;
	}
	if(texindex == -1) {
		return nullptr;
	}

	return &editor->textures()[texindex];
	*/
	return nullptr;
}


irect2d poly2d::uv(const irect2d &aabb) const
{
	irect2d rect = aabb;
	if(texscale != 0) {
		float scale = static_cast<float>(texscale * gl::ctx::GRID_SPACING);
		glm::vec2 mins = { 0.0f, 0.0f };
		glm::vec2 maxs = { scale, scale };

		rect.mins = { 0.0f, 0.0f };
		rect.maxs = { scale, scale };
	}
	return rect;
}


irect2d poly2d::uv() const
{
	return uv(m_aabb);
}
