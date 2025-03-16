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
private:
	GLuint m_program;
	GLuint m_vtxbuf;
	GLuint m_vao;
public:
	[[nodiscard]] static glm::i32vec2 Snap(const glm::vec2 &pt)
	{
		int32_t x = round(pt.x / SPACING) * SPACING;
		int32_t y = round(pt.y / SPACING) * SPACING;

		return glm::i32vec2(x, y);
	}
};


#endif 
