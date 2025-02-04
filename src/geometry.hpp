
#ifndef _GEOMETRY_HPP
#define _GEOMETRY_HPP

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <wx/debug.h>

using Point2D = glm::vec2;
using Color = glm::vec4;
using Matrix4 = glm::mat4;

constexpr Color BLACK = Color(0.0f, 0.0f, 0.0f, 1.0f);
constexpr Color WHITE = Color(1.0f, 1.0f, 1.0f, 1.0f);
constexpr Color RED = Color(1.0f, 0.0f, 0.0f, 1.0f);
constexpr Color BLUE = Color(0.0f, 0.0f, 1.0f, 1.0f);
constexpr Color GREEN = Color(0.0f, 1.0f, 0.0f, 1.0f);

struct Rect2D
{
	Rect2D() = default;
	Rect2D(const Point2D &mins, const Point2D &maxs)
		: mins(mins),
		  maxs(maxs)
	{
		wxASSERT(maxs.x >= mins.x);
		wxASSERT(maxs.y >= mins.y);
	}
	void Offset(const Point2D &delta) 
	{
		mins += delta;
		maxs += delta;
	}
	
	bool Intersects(const Rect2D &other) const 
	{
		return mins.x < other.maxs.x && other.mins.x < maxs.x &&
		       mins.y < other.maxs.y && other.mins.y < maxs.y;
	};
	bool Contains(const Point2D &pt) const 
	{
		return pt.x > mins.x && pt.y > mins.y &&
		       pt.x < maxs.x && pt.y < maxs.y;
	};

	void Inset(float x, float y)
	{
		Point2D delta = { x * 0.5, y * 0.5 };
		mins = mins + delta;
		maxs = maxs - delta;
	}

	Point2D GetLeftTop() const { return mins; }
	Point2D GetRightTop() const { return { maxs.x, mins.y }; }
	Point2D GetLeftBottom() const { return { mins.x, maxs.y }; }
	Point2D GetRightBottom() const { return maxs; }

	void FitPoints(const Point2D pts[], size_t npts);
	void SetRight(float r) { maxs.x = r; }
	void SetLeft(float l) { mins.x = l; }
	void SetBottom(float b) { maxs.y = b; }
	void SetTop(float t) { mins.y = t; }
	Point2D mins, maxs;
};

static bool Intersects(const Rect2D &a, const Rect2D &b)
{
	return a.mins.x < b.maxs.x && b.mins.x < a.maxs.x &&
	       a.mins.y < b.maxs.y && b.mins.y < a.maxs.y;
}

struct Plane2D
{
	Plane2D() = default;
	Plane2D(const Point2D &start, const Point2D &end);
	float SignedDistance(const Point2D &p) const;
	bool Line(const Point2D &a, const Point2D &b, Point2D &out) const;
	void Flip();
	void Offset(const Point2D &pt);
	void Clip(const std::vector<Point2D> &points, std::vector<Point2D> &out);
	bool operator==(const Plane2D &other) const;
	/* Ax + By + C = 0 */
	float a, b, c;
};

struct GLTexture;

struct ConvexPolygon
{
	ConvexPolygon(const Rect2D &rect);
	void MoveBy(Point2D delta);
	void Slice(Plane2D plane);
	void ImposePlane(Plane2D plane, std::vector<Point2D> &out) const;
	static bool AllPointsBehind(const Plane2D &plane, const Point2D points[], size_t npoints);
	bool AllPointsBehind(const Plane2D &plane) const;
	void ResizeAABB();
	void PurgePlanes();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const Rect2D &rect) const;
	bool Contains(const Point2D &pt) const;
	GLTexture *GetTexture() const;
	Rect2D GetUV() const;
	Rect2D GetUV(const Rect2D &aabb) const;

	Rect2D aabb;
	std::vector<Plane2D> planes;
	/* internal representation of polygon */
	std::vector<Point2D> points;

	size_t texindex = -1;
	int texscale; 
};

#endif
