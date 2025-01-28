#ifndef _GLBACKGROUNDGRID_HPP
#define _GLBACKGROUNDGRID_HPP

#include <glad/gl.h>
#include "src/geometry.hpp"

class GLBackgroundGrid
{
public:
	GLBackgroundGrid();
	~GLBackgroundGrid();
	void SetMatrices(const Matrix4 &proj, const Matrix4 &view);
	void DrawGrid();
	constexpr static int SPACING = 50;
	static void Snap(Point2D &pt)
	{
		pt.x = round(pt.x / SPACING) * SPACING;
		pt.y = round(pt.y / SPACING) * SPACING;
	}
private:
	GLuint m_program;
	GLuint m_vtxbuf;
	GLuint m_vao;
};


#endif 
