#include <glad/gl.h>
#include <wx/wx.h>
#include "src/geometry.hpp"
#include "src/glcontext.hpp"


static GLuint CompileShaders(const char *fs_src, const char *vs_src)
{
	GLuint vs;
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src, NULL);
	glCompileShader(vs);

	GLint success;
	char infolog[512];
	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(vs, 512, NULL, infolog);
		wxMessageBox(infolog);
		return 0;
	}

	GLuint fs;
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(fs, 512, NULL, infolog);
		wxMessageBox(infolog);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infolog);
		wxMessageBox(infolog);
		return 0;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}


static wxGLContextAttrs *ContextAttrs_Core33()
{
	static wxGLContextAttrs ctxattrs;
	ctxattrs.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();
	return &ctxattrs;
}


void GLContext::Clear(const Color &color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}


void GLContext::SetMatrices(const Matrix4 &proj, const Matrix4 &view)
{
	glUseProgram(m_program);
	Matrix4 mvp = proj * view;
	int pos = glGetUniformLocation(m_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));

	m_grid->SetMatrices(proj, view);
}

void GLContext::DrawElements()
{
	m_grid->DrawGrid();
	
	glUseProgram(m_program);
	glBindVertexArray(m_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
}


void GLContext::AddQuad(const Point2D q[4], const Color &color)
{
	for(size_t i = 0; i < 4; i++) {
		m_points.push_back(q[i]);
		m_colors.push_back(color);
	}

	size_t i = m_points.size() - 1;
	m_indices.push_back(i - 3);
	m_indices.push_back(i - 2);
	m_indices.push_back(i - 1);
	m_indices.push_back(i - 2);
	m_indices.push_back(i - 0);
	m_indices.push_back(i - 1);
}


void GLContext::AddRect(const Rect2D &rect, const Color &color)
{
	Point2D q[4] = {
		rect.GetLeftTop(),
		rect.GetRightTop(),
		rect.GetLeftBottom(),
		rect.GetRightBottom()
	};

	AddQuad(q, color);
}


void GLContext::AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color)
{
	Point2D delta = glm::normalize(b - a);
	Point2D normal = { -delta.y, delta.x };

	normal *= thickness * 0.5;

	Point2D q[4] = {
		a - normal,
		a + normal,
		b - normal,
		b + normal
	};

	AddQuad(q, color);
}


void GLContext::ClearBuffers()
{
	m_points.clear();
	m_indices.clear();
	m_colors.clear();
}


void GLContext::CopyBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, m_points.size() * sizeof(Point2D), m_points.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m_colbuf);
	glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(Color), m_colors.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);
}


GLContext::GLContext(wxGLCanvas *parent)
	: wxGLContext(parent, nullptr, ContextAttrs_Core33())
{
	wxASSERT(IsOK());

	SetCurrent(*parent);
	int version = gladLoaderLoadGL();
	wxASSERT(version != 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_MULTISAMPLE);

	m_grid = new GLBackgroundGrid(this);
	
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


	m_program = CompileShaders(fs_src, vs_src);

	glGenBuffers(1, &m_vtxbuf);
	glGenBuffers(1, &m_colbuf);
	glGenBuffers(1, &m_idxbuf);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_colbuf);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);

	CopyBuffers();
}

GLContext::~GLContext()
{
	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteBuffers(1, &m_colbuf);
	glDeleteBuffers(1, &m_idxbuf);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}


GLBackgroundGrid::GLBackgroundGrid(GLContext *context)
	: m_context(context)
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

	m_program = CompileShaders(fs_src, vs_src);
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
