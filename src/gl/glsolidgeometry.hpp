#ifndef _GLSOLIDGEOMETRY_HPP
#define _GLSOLIDGEOMETRY_HPP

#include <glad/gl.h>
#include "src/singleton.hpp"
#include "src/geometry.hpp"

struct SolidVertex
{
	glm::vec2 position;
	glm::vec4 color;
};

class GLSolidGeometry : Immobile
{
public:
	void Init();
	~GLSolidGeometry();
	void ClearBuffers();
	void CopyBuffers();
	void DrawElements();
	void SetMatrices(const glm::mat4 &proj, const glm::mat4 &view);
	void AddRect(const glm::vec2 &mins, const glm::vec2 &maxs, const glm::vec4 &color);
	void AddLine(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color);
private:
	void AddQuad(const glm::vec2 q[4], const glm::vec4 &color);
private:
	GLuint m_vtxbuf;
	GLuint m_idxbuf;
	GLuint m_vao;
	GLuint m_program;
private:
	std::vector<SolidVertex> m_vtx;
	std::vector<GLuint> m_idx;
};

#endif
