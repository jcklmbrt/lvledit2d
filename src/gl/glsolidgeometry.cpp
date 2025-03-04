
#include <glm/gtc/type_ptr.hpp>
#include "src/gl/glcontext.hpp"
#include "src/gl/glsolidgeometry.hpp"


void GLSolidGeometry::Init()
{
	static const char *vs_src = R"(
		#version 330 core

		layout (location = 0) in vec2 pos;
		layout (location = 1) in vec4 color;

		out vec4 vtx_color;

		uniform mat4 mvp;

		void main()
		{
			gl_Position = mvp * vec4(pos, 0.0, 1.0);
			vtx_color = color;
		}
	)";

	static const char *fs_src = R"(
		#version 330 core

		out vec4 FragColor;
		in vec4 vtx_color;

		void main()
		{
			FragColor = vtx_color;
		}
	)";

	m_program = GLContext::CompileShaders(fs_src, vs_src);

	glGenBuffers(1, &m_vtxbuf);
	glGenBuffers(1, &m_idxbuf);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (void *)offsetof(SolidVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SolidVertex), (void *)offsetof(SolidVertex, color));
	glEnableVertexAttribArray(1);

	CopyBuffers();
}


GLSolidGeometry::~GLSolidGeometry()
{
	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteBuffers(1, &m_idxbuf);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}


void GLSolidGeometry::CopyBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, m_vtx.size() * sizeof(SolidVertex), m_vtx.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idx.size() * sizeof(GLuint), m_idx.data(), GL_STREAM_DRAW);
}


void GLSolidGeometry::DrawElements()
{
	glUseProgram(m_program);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glDrawElements(GL_TRIANGLES, m_idx.size(), GL_UNSIGNED_INT, 0);
}


void GLSolidGeometry::AddQuad(const glm::vec2 q[4], const glm::vec4 &color)
{
	SolidVertex vtx;
	vtx.color = color;

	for(size_t i = 0; i < 4; i++) {
		vtx.position = q[i];
		m_vtx.push_back(vtx);
	}

	size_t i = m_vtx.size() - 1;
	m_idx.push_back(i - 3);
	m_idx.push_back(i - 2);
	m_idx.push_back(i - 1);
	m_idx.push_back(i - 2);
	m_idx.push_back(i - 0);
	m_idx.push_back(i - 1);
}


void GLSolidGeometry::AddRect(const glm::vec2 &mins, const glm::vec2 &maxs, const glm::vec4 &color)
{
	glm::vec2 q[4] = {
		{ mins.x, mins.y },
		{ maxs.x, mins.y },
		{ mins.x, maxs.y },
		{ maxs.x, maxs.y }
	};

	AddQuad(q, color);
}


void GLSolidGeometry::AddLine(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color)
{
	glm::vec2 delta = glm::normalize(b - a) * thickness * 0.5f;
	glm::vec2 normal = { -delta.y, delta.x };

	glm::vec2 q[4] = {
		(a - delta) - normal,
		(a - delta) + normal,
		(b + delta) - normal,
		(b + delta) + normal
	};

	AddQuad(q, color);
}


void GLSolidGeometry::ClearBuffers()
{
	m_idx.clear();
	m_vtx.clear();
}


void GLSolidGeometry::SetMatrices(const glm::mat4 &proj, const glm::mat4 &view)
{
	glUseProgram(m_program);
	glm::mat4 mvp = proj * view;
	int pos = glGetUniformLocation(m_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));
}

