#ifndef _GLBACKGROUNDGRID_HPP
#define _GLBACKGROUNDGRID_HPP

#include <glad/gl.h>
#include "src/geometry.hpp"

class GLBackgroundGrid
{
public:
	void Init();
	~GLBackgroundGrid();
	void SetMatrices(const glm::mat4 &proj, const glm::mat4 &view);
	void DrawGrid();
	constexpr static int SPACING = 1;
	static void Snap(glm::vec2 &pt);
	static void Snap(glm::i32vec2 &pt);
private:
	GLuint m_program;
	GLuint m_vtxbuf;
	GLuint m_vao;
};


inline void GLBackgroundGrid::Snap(glm::vec2 &pt)
{
	pt.x = round(pt.x / SPACING) * SPACING;
	pt.y = round(pt.y / SPACING) * SPACING;
}

inline void GLBackgroundGrid::Snap(glm::i32vec2 &pt)
{
	pt.x = round(pt.x / SPACING) * SPACING;
	pt.y = round(pt.y / SPACING) * SPACING;
}

#endif 
