#include "src/gl/glbackgroundgrid.hpp"
#include "src/gl/glcontext.hpp"


void GLBackgroundGrid::Init()
{
	static const char *fs_src = R"(
		#version 330 core
		in vec4 s_pos;
		out vec4 FragColor;
		uniform float spacing;
		uniform float zoom;

		void main()
		{
			vec4 fg_color = vec4(0.5, 0.5, 0.5, 0.2);
			vec4 bg_color = vec4(0.0);

			float width = 1.0 / zoom;
			vec2 gridpos = mod(s_pos.xy, spacing);

			float is_line = step(gridpos.x, width) + step(gridpos.y, width);

			FragColor = mix(bg_color, fg_color, min(is_line, 1.0));
		}
	)";

	static const char *vs_src = R"(
		#version 330 core
		layout (location = 0) in vec2 pos;

		out vec4 s_pos;

		uniform mat4 inv_mvp;

		void main()
		{
			gl_Position = vec4(pos, 0.0, 1.0);
			s_pos = inv_mvp * vec4(pos, 0.0, 1.0);
		}
	)";

	m_program = GLContext::CompileShaders(fs_src, vs_src);
	wxASSERT(m_program);
	/* fullscreen rect. don't bother with EBO. */
	static float vertices[] = {
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f
	};

	glUseProgram(m_program);
	GLint spacing = glGetUniformLocation(m_program, "spacing");
	glUniform1f(spacing, static_cast<float>(SPACING));

	glGenBuffers(1, &m_vtxbuf);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
}


GLBackgroundGrid::~GLBackgroundGrid()
{
	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}


void GLBackgroundGrid::SetMatrices(const Matrix4 &proj, const Matrix4 &view)
{
	glUseProgram(m_program);
	Matrix4 inv_view = glm::inverse(view);
	Matrix4 inv_proj = glm::inverse(proj);
	Matrix4 inv_mvp = inv_view * inv_proj;
	int mvppos = glGetUniformLocation(m_program, "inv_mvp");
	glUniformMatrix4fv(mvppos, 1, GL_FALSE, glm::value_ptr(inv_mvp));
	int zoompos = glGetUniformLocation(m_program, "zoom");
	glUniform1f(zoompos, view[0][0]);
}


void GLBackgroundGrid::DrawGrid()
{
	glUseProgram(m_program);
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
