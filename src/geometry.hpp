
#ifndef _GEOMETRY_HPP
#define _GEOMETRY_HPP

#include <vector>
#include <glm/glm.hpp>
#include <wx/debug.h>

using glm_vec2 = glm::vec2;

constexpr glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 RED = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 BLUE = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 GREEN = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

constexpr int RECT2D_INSIDE = 0;
constexpr int RECT2D_LEFT = 1;
constexpr int RECT2D_RIGHT = 2;
constexpr int RECT2D_BOTTOM = 4;
constexpr int RECT2D_TOP = 8;

struct Rect2D
{
	Rect2D() = default;
	Rect2D(const glm::i32vec2 &mins, const glm::i32vec2 &maxs)
		: mins(mins),
		  maxs(maxs)
	{
		wxASSERT(maxs.x >= mins.x);
		wxASSERT(maxs.y >= mins.y);
	}

	void Offset(const glm::i32vec2 &delta) 
	{
		mins += delta;
		maxs += delta;
	}
	
	bool Intersects(const Rect2D &other) const 
	{
		return mins.x < other.maxs.x && other.mins.x < maxs.x &&
		       mins.y < other.maxs.y && other.mins.y < maxs.y;
	};

	int GetOutCode(const glm::i32vec2 &p) const
        {
		return (((p.x < mins.x) ? RECT2D_LEFT : 0) +
		        ((p.x > maxs.x) ? RECT2D_RIGHT : 0) +
		        ((p.y < mins.y) ? RECT2D_TOP : 0)  +
		        ((p.y > maxs.y) ? RECT2D_BOTTOM : 0));
	}

	bool Contains(const glm::i32vec2 &pt) const 
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	};

	void Inset(int32_t x, int32_t y)
	{
		glm::i32vec2 delta = { x / 2, y / 2 };
		mins = mins + delta;
		maxs = maxs - delta;
	}

	glm::i32vec2 GetLeftTop() const { return mins; }
	glm::i32vec2 GetRightTop() const { return { maxs.x, mins.y }; }
	glm::i32vec2 GetLeftBottom() const { return { mins.x, maxs.y }; }
	glm::i32vec2 GetRightBottom() const { return maxs; }

	glm::i32vec2 GetSize() const { return maxs - mins; }
	glm::i32vec2 GetCenter() const { return maxs - (GetSize() / 2); } 

	void FitPoints(const glm_vec2 pts[], size_t npts);

	glm::i32vec2 mins, maxs;
};

static bool Intersects(const Rect2D &a, const Rect2D &b)
{
	return a.mins.x < b.maxs.x && b.mins.x < a.maxs.x &&
	       a.mins.y < b.maxs.y && b.mins.y < a.maxs.y;
}

struct Plane2D
{
	Plane2D() = default;
	Plane2D(const glm::i32vec2 &start, const glm::i32vec2 &end);
	int32_t SignedDistance(const glm::i32vec2 &p) const;
	bool Line(const glm::vec2 &a, const glm::vec2 &b, glm::vec2 &out) const;
	void Flip();
	void Offset(const glm::i32vec2 &pt);
	void Transform(const glm::mat3 &t);
	void Scale(const glm_vec2 &origin, const glm_vec2 &scale);
	void Clip(const std::vector<glm_vec2> &points, std::vector<glm_vec2> &out);
	bool operator==(const Plane2D &other) const;
	/* Ax + By + C = 0 */
	int32_t a, b, c;
};

struct GLTexture;

struct ConvexPolygon
{
	ConvexPolygon(const Rect2D &rect);
	void Transform(const glm::mat3 &t);
	void Slice(Plane2D plane);
	void ImposePlane(Plane2D plane, std::vector<glm_vec2> &out) const;
	static bool AllPointsBehind(const Plane2D &plane, const glm::vec2 points[], size_t npoints);
	static bool AllPointsBehind(const Plane2D &plane, const glm::i32vec2 points[], size_t npoints);
	bool AllPointsBehind(const Plane2D &plane) const;
	void ResizeAABB();
	void PurgePlanes();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const Rect2D &rect) const;
	bool Contains(const glm_vec2 &pt) const;
	GLTexture *GetTexture() const;
	Rect2D GetUV() const;
	Rect2D GetUV(const Rect2D &aabb) const;

	Rect2D aabb;
	std::vector<Plane2D> planes;
	/* internal representation of polygon */
	std::vector<glm_vec2> points;

	size_t texindex = -1;
	int texscale; 
};

#endif
