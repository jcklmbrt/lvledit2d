
#ifndef _GEOMETRY_HPP
#define _GEOMETRY_HPP

#include "glm/fwd.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <wx/debug.h>

constexpr glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 RED = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 BLUE = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 LIGHT_BLUE = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
constexpr glm::vec4 PINK = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 GREEN = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

constexpr int RECT2D_INSIDE = 0;
constexpr int RECT2D_LEFT = 1;
constexpr int RECT2D_RIGHT = 2;
constexpr int RECT2D_BOTTOM = 4;
constexpr int RECT2D_TOP = 8;
constexpr int RECT2D_OUTX = RECT2D_LEFT | RECT2D_RIGHT;
constexpr int RECT2D_OUTY = RECT2D_BOTTOM | RECT2D_TOP;

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
	
	bool Intersects(const Rect2D &other) const 
	{
		return mins.x < other.maxs.x && other.mins.x < maxs.x &&
		       mins.y < other.maxs.y && other.mins.y < maxs.y;
	};

	int GetOutCode(const glm::vec2 &p) const
	{
		return (((p.x < mins.x) ? RECT2D_LEFT : 0) +
			((p.x > maxs.x) ? RECT2D_RIGHT : 0) +
			((p.y < mins.y) ? RECT2D_TOP : 0) +
			((p.y > maxs.y) ? RECT2D_BOTTOM : 0));
	}

	int GetOutCode(const glm::i32vec2 &p) const
        {
		return (((p.x < mins.x) ? RECT2D_LEFT : 0) +
		        ((p.x > maxs.x) ? RECT2D_RIGHT : 0) +
		        ((p.y < mins.y) ? RECT2D_TOP : 0)  +
		        ((p.y > maxs.y) ? RECT2D_BOTTOM : 0));
	}

	bool Contains(const glm::vec2 &pt) const
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	}

	bool Contains(const glm::i32vec2 &pt) const 
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	};

	glm::i32vec2 GetSize() const { return maxs - mins; }
	void FitPoints(const glm::vec2 pts[], size_t npts);

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
	float SignedDistance(const glm::vec2 &p) const;
	void Normalize();
	void Flip();
	void Offset(const glm::i32vec2 &pt);
	void Scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom);
	void Clip(const std::vector<glm::vec2> &points, std::vector<glm::vec2> &out);
	bool operator==(const Plane2D &other) const;
	/* Ax + By + C = 0 */
	int32_t a, b, c;
};

class GLTexture;
struct ActTexture;

class ConvexPolygon
{
public:
	ConvexPolygon(const Rect2D &rect);
	void Offset(const glm::i32vec2 &delta);
	void Scale(const glm::i32vec2 &origin, const glm::i32vec2 &numer, const glm::i32vec2 &denom);
	void Slice(Plane2D plane);
	void ImposePlane(Plane2D plane, std::vector<glm::vec2> &out) const;
	static bool AllPointsBehind(const Plane2D &plane, const glm::vec2 points[], size_t npoints);
	static bool AllPointsBehind(const Plane2D &plane, const glm::i32vec2 points[], size_t npoints);
	bool AllPointsBehind(const Plane2D &plane) const;
	void ResizeAABB();
	void PurgePlanes();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const Rect2D &rect) const;
	bool Contains(const glm::vec2 &pt) const;
	GLTexture *GetTexture() const;
	Rect2D GetUV() const;
	Rect2D GetUV(const Rect2D &aabb) const;
private:
	Rect2D m_aabb;
	std::vector<Plane2D> m_planes;
	/* internal representation of polygon */
	std::vector<glm::vec2> m_points;

	size_t m_texindex = -1;
	int m_texscale;
public:
	inline void SetTexture(size_t index, int scale)
	{
		m_texindex = index;
		m_texscale = scale;
	}

	inline Rect2D GetAABB() const 
	{
		return m_aabb;
	}

	inline const std::vector<glm::vec2> &GetPoints() const
	{
		return m_points;
	}

	inline const std::vector<Plane2D> &GetPlanes() const
	{
		return m_planes;
	}
};

#endif
