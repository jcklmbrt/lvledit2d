
#ifndef _GEOMETRY_HPP
#define _GEOMETRY_HPP

#include "glm/fwd.hpp"
#include <vector>
#include <glm/glm.hpp>

constexpr glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 RED = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 BLUE = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 LIGHT_BLUE = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
constexpr glm::vec4 PINK = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 PASTEL_PINK = glm::vec4(1.0f, 0.85f, 0.85f, 1.0f);
constexpr glm::vec4 PASTEL_BLUE = glm::vec4(0.7f, 0.95f, 0.95f, 1.0f);
constexpr glm::vec4 GREEN = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

struct irect2d
{
	enum outcode : int {
		INSIDE = 0,
		LEFT = 1,
		RIGHT = 2,
		BOTTOM = 4,
		TOP = 8,
		OUTX = LEFT | RIGHT,
		OUTY = BOTTOM | TOP
	};
	irect2d() = default;
	
	irect2d(const glm::i32vec2 &a, const glm::i32vec2 &b)
	{
		mins.x = std::min(a.x, b.x);
		mins.y = std::min(a.y, b.y);

		maxs.x = std::max(a.x, b.x);
		maxs.y = std::max(a.y, b.y);
	}
	
	bool intersects(const irect2d &other) const 
	{
		return mins.x < other.maxs.x && other.mins.x < maxs.x &&
		       mins.y < other.maxs.y && other.mins.y < maxs.y;
	};

	int outcode(const glm::vec2 &p) const
	{
		return (((p.x < mins.x) ? LEFT : 0) +
			((p.x > maxs.x) ? RIGHT : 0) +
			((p.y < mins.y) ? TOP : 0) +
			((p.y > maxs.y) ? BOTTOM : 0));
	}

	int outcode(const glm::i32vec2 &p) const
        {
		return (((p.x < mins.x) ? LEFT : 0) +
		        ((p.x > maxs.x) ? RIGHT : 0) +
		        ((p.y < mins.y) ? TOP : 0)  +
		        ((p.y > maxs.y) ? BOTTOM : 0));
	}

	bool contains(const glm::vec2 &pt) const
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	}

	bool contains(const glm::i32vec2 &pt) const 
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	};

	glm::i32vec2 size() const { return maxs - mins; }
	void fit(const glm::vec2 pts[], size_t npts);

	glm::i32vec2 mins, maxs;
};


struct iline2d {
	iline2d() = default;
	iline2d(const glm::i32vec2 &start, const glm::i32vec2 &end);
	int32_t distnumer(const glm::i32vec2 &p) const;
	float distnumer(const glm::vec2 &p) const;
	void normalize();
	void flip();
	void offset(const glm::i32vec2 &pt);
	void scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom);
	void clip(const std::vector<glm::vec2> &points, std::vector<glm::vec2> &out);
	bool operator==(const iline2d &other) const;
	// Ax + By + C = 0
	int32_t a, b, c;
};

namespace gl { struct texture; }
namespace act { struct texture; }

struct poly2d {
	poly2d(const irect2d &rect);
	void offset(const glm::i32vec2 &delta);
	void scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom);
	void slice(iline2d plane);
	void addline(iline2d plane, std::vector<glm::vec2> &out) const;
	static bool allptsbehind(const iline2d &plane, const glm::vec2 points[], size_t npoints);
	static bool allptsbehind(const iline2d &plane, const glm::i32vec2 points[], size_t npoints);
	bool allptsbehind(const iline2d &plane) const;
	void fitaabb();
	void fitlines();
	bool intersects(const poly2d &other) const;
	bool intersects(const irect2d &rect) const;
	bool contains(const glm::vec2 &pt) const;
	gl::texture *texture() const;
	irect2d uv() const;
	irect2d uv(const irect2d &aabb) const;
private:
	// data for collisions
	irect2d m_aabb;
	std::vector<iline2d> m_lines;
	// data for rendering
	std::vector<glm::vec2> m_points;
public:
	// texture data
	size_t texindex = -1;
	int texscale = 1;
public:
	inline const irect2d &aabb() const { return m_aabb; }
	inline const std::vector<glm::vec2> &points() const { return m_points; }
	inline const std::vector<iline2d> &planes() const { return m_lines; }
};

#endif
