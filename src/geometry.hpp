
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

class Rect2D
{
public:
	Rect2D() = default;
	Rect2D(const Point2D &mins, const Point2D &maxs)
		: m_mins(mins),
		  m_maxs(maxs)
	{
		wxASSERT(maxs.x >= mins.x);
		wxASSERT(maxs.y >= mins.y);
	}
	void Offset(const Point2D &delta) 
	{
		m_mins += delta;
		m_maxs += delta;
	}
	
	Point2D GetLeftTop() const { return m_mins; }
	Point2D GetRightBottom() const { return m_maxs; }
	Point2D GetLeftBottom() const { return { m_mins.x, m_maxs.y }; }
	Point2D GetRightTop() const { return { m_maxs.x, m_mins.y }; }
	Point2D GetSize() const { return m_maxs - m_mins; }
	bool Intersects(const Rect2D &other) const 
	{
		return m_mins.x < other.m_maxs.x && other.m_mins.x < m_maxs.x &&
		       m_mins.y < other.m_maxs.y && other.m_mins.y < m_maxs.y;
	};
	bool Contains(const Point2D &pt) const 
	{
		return pt.x > m_mins.x && pt.y > m_mins.y &&
		       pt.x < m_maxs.x && pt.y < m_maxs.y;
	};

	void Inset(float x, float y)
	{
		Point2D delta = { x * 0.5, y * 0.5 };
		m_mins = m_mins + delta;
		m_maxs = m_maxs - delta;
	}
	void FitPoints(const Point2D pts[], size_t npts);
	void SetMins(const Point2D &mins) { m_mins = mins; }
	void SetMaxs(const Point2D &maxs) { m_maxs = maxs; }
	void SetRight(float r) { m_maxs.x = r; }
	void SetLeft(float l) { m_mins.x = l; }
	void SetBottom(float b) { m_maxs.y = b; }
	void SetTop(float t) { m_mins.y = t; }
private:
	Point2D m_mins;
	Point2D m_maxs;
};

class Plane2D
{
public:
	Plane2D() = default;
	Plane2D(const Point2D &start, const Point2D &end);
	float SignedDistance(const Point2D &p) const;
	bool Line(const Point2D &a, const Point2D &b, Point2D &out) const;
	void Flip();
	void Offset(const Point2D &pt);
	void Clip(const std::vector<Point2D> &points, std::vector<Point2D> &out);
	bool operator==(const Plane2D &other) const;
private:
	/* Ax + By + C = 0 */
	float m_a, m_b, m_c;
};

class Texture;

class ConvexPolygon
{
public:
	ConvexPolygon(const Rect2D &rect);
	void MoveBy(Point2D delta);
	void Slice(Plane2D plane);
	void ImposePlane(Plane2D plane, std::vector<Point2D> &out) const;
	static bool AllPointsBehind(const Plane2D &plane, const Point2D points[], size_t npoints);
	bool AllPointsBehind(const Plane2D &plane) const;
	const Rect2D &GetAABB() const { return m_aabb; }
	const std::vector<Plane2D> &GetPlanes() const { return m_planes; }
	const std::vector<Point2D> &GetPoints() const { return m_points; }
	void ResizeAABB();
	void PurgePlanes();
	bool Intersects(const ConvexPolygon &other) const;
	bool Intersects(const Rect2D &rect) const;
	bool Contains(const Point2D &pt) const;
	Texture *GetTexture() const;
	void SetTexture(size_t i) { m_texture = i; }
private:
	Rect2D m_aabb;
	std::vector<Plane2D> m_planes;
	/* internal representation of polygon */
	std::vector<Point2D> m_points;

	size_t m_texture = -1;
};

#endif
