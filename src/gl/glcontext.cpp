#include <cstddef>
#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtc/type_ptr.hpp>

#include "src/gl/glcontext.hpp"
#include "src/geometry.hpp"

#include "res/icon_atlas.png.hpp"
namespace ia = icon_atlas;

#include "res/tahoma12.png.hpp"

extern void set_window_icon(unsigned char data[], int width, int height);

static GLuint compileshaders(const char *fs_src, const char *vs_src)
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
		//wxMessageBox(infolog);
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
		//wxMessageBox(infolog);
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
		//wxMessageBox(infolog);
		return 0;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}


void gl::ctx::clear(const glm::vec4 &color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl::ctx::setmatrices(const glm::mat4 &proj, const glm::mat4 &view)
{
	glUseProgram(m_solid_program);
	glm::mat4 mvp = proj * view;
	int pos = glGetUniformLocation(m_solid_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));

	glUseProgram(m_texture_program);
	pos = glGetUniformLocation(m_texture_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));

	glUseProgram(m_grid_program);
	glm::mat4 inv_view = glm::inverse(view);
	glm::mat4 inv_proj = glm::inverse(proj);
	glm::mat4 inv_mvp = inv_view * inv_proj;
	int mvppos = glGetUniformLocation(m_grid_program, "inv_mvp");
	glUniformMatrix4fv(mvppos, 1, GL_FALSE, glm::value_ptr(inv_mvp));
	int zoompos = glGetUniformLocation(m_grid_program, "zoom");
	glUniform1f(zoompos, view[0][0]);
}

void gl::ctx::drawgrid()
{
	// background grid, a single rect
	glUseProgram(m_grid_program);
	glBindVertexArray(m_grid_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

}

void gl::ctx::end()
{
	// draw all texture batches
	glUseProgram(m_texture_program);
	glBindVertexArray(m_vao);
	for(auto &[texid, vtc] : m_texture_batches) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
		glBufferData(GL_ARRAY_BUFFER, vtc.vtx.size() * sizeof(vtx), vtc.vtx.data(), GL_STREAM_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vtc.idx.size() * sizeof(GLuint), vtc.idx.data(), GL_STREAM_DRAW);

		glBindTexture(GL_TEXTURE_2D, texid);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
		glDrawElements(GL_TRIANGLES, vtc.idx.size(), GL_UNSIGNED_INT, 0);
	}

	glBindTexture(GL_TEXTURE_2D, m_font_atlas);

	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, m_solid_vtx.size() * sizeof(vtx), m_solid_vtx.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_solid_idx.size() * sizeof(GLuint), m_solid_idx.data(), GL_STREAM_DRAW);

	// draw solid geometry
	glUseProgram(m_solid_program);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glDrawElements(GL_TRIANGLES, m_solid_idx.size(), GL_UNSIGNED_INT, 0);
}


void gl::ctx::begin()
{
	m_solid_idx.clear();
	m_solid_vtx.clear();
	m_texture_batches.clear();
}


void gl::ctx::quad(const glm::vec2 q[4], const glm::vec4 &color)
{
	gl::vtx vtx;
	vtx.color = color;

	begin();

	for(size_t i = 0; i < 4; i++) {
		vtx.pos = q[i];
		vtx.uv = { 0.0f, 0.0f };
		m_solid_vtx.push_back(vtx);
	}

	size_t i = m_solid_vtx.size() - 1;
	m_solid_idx.push_back(i - 3);
	m_solid_idx.push_back(i - 2);
	m_solid_idx.push_back(i - 1);
	m_solid_idx.push_back(i - 2);
	m_solid_idx.push_back(i - 0);
	m_solid_idx.push_back(i - 1);

	end();
}

void gl::ctx::rect(const glm::vec2 &mins, const glm::vec2 &maxs, const glm::vec4 &color)
{
	glm::vec2 q[4] = {
		{ mins.x, mins.y },
		{ maxs.x, mins.y },
		{ mins.x, maxs.y },
		{ maxs.x, maxs.y }
	};

	quad(q, color);
}


void gl::ctx::line(const glm::vec2 &a, const glm::vec2 &b, float thickness, const glm::vec4 &color)
{
	glm::vec2 delta = glm::normalize(b - a) * thickness * 0.5f;
	glm::vec2 normal = { -delta.y, delta.x };

	glm::vec2 q[4] = {
		(a - delta) - normal,
		(a - delta) + normal,
		(b + delta) - normal,
		(b + delta) + normal
	};

	quad(q, color);
}

static gl::vtx setvtx(const glm::vec2 &pt, const glm::vec4 &color, const irect2d &aabb)
{
	glm::vec2 mins = aabb.mins;
	glm::vec2 size = aabb.maxs - aabb.mins;

	gl::vtx vtx;
	vtx.pos = pt;
	vtx.color = color;
	vtx.uv = (pt - mins) / size;
	return vtx;
}


void gl::ctx::poly(const glm::vec2 pts[], size_t npts, const irect2d &uv, gl::texture &texture, const glm::vec4 &color)
{
	begin();

	if(!glIsTexture(texture.gltex())) {
		texture.init_gltex();
	}

	vtx start = setvtx(pts[0], color, uv);

	texturebatch &vtc = m_texture_batches[texture.gltex()];

	vtc.vtx.push_back(start);
	size_t start_idx = vtc.vtx.size() - 1;

	for(size_t i = 1; i < npts; i++) {
		glm::vec2 a = pts[i];
		glm::vec2 b = pts[(i + 1) % npts];
		vtc.vtx.push_back(setvtx(a, color, uv));
		vtc.vtx.push_back(setvtx(b, color, uv));

		vtc.idx.push_back(start_idx);
		vtc.idx.push_back(start_idx + (i * 2) - 1);
		vtc.idx.push_back(start_idx + (i * 2));
	}
	end();
}


gl::ctx::~ctx()
{
	// delete background grid
	glDeleteBuffers(1, &m_grid_vtxbuf);
	glDeleteVertexArrays(1, &m_grid_vao);
	glDeleteProgram(m_grid_program);

	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteBuffers(1, &m_idxbuf);
	glDeleteVertexArrays(1, &m_vao);

	glDeleteProgram(m_solid_program);
	glDeleteProgram(m_texture_program);
}


void gl::ctx::icon(const glm::vec2 &pos, ia::position uv, const glm::vec4 &color)
{
	begin();
	texturebatch &vtc = m_texture_batches[m_icon_atlas];

	gl::vtx vtx;
	vtx.color = color;

	// top left
	vtx.pos = pos;
	vtx.uv.x = static_cast<float>(uv.x) / m_icon_atlas_width;
	vtx.uv.y = static_cast<float>(uv.y) / m_icon_atlas_height;
	vtc.vtx.push_back(vtx);

	// top right
	vtx.pos.x = pos.x + uv.w;
	vtx.pos.y = pos.y;
	vtx.uv.x = static_cast<float>(uv.x + uv.w) / m_icon_atlas_width;
	vtx.uv.y = static_cast<float>(uv.y)        / m_icon_atlas_height;
	vtc.vtx.push_back(vtx);

	// bottom left
	vtx.pos.x = pos.x;
	vtx.pos.y = pos.y + uv.h;
	vtx.uv.x = static_cast<float>(uv.x) / m_icon_atlas_width;
	vtx.uv.y = static_cast<float>(uv.y + uv.h) / m_icon_atlas_height;
	vtc.vtx.push_back(vtx);

	// bottom right
	vtx.pos.x = pos.x + uv.w;
	vtx.pos.y = pos.y + uv.h;
	vtx.uv.x = static_cast<float>(uv.x + uv.w) / m_icon_atlas_width;
	vtx.uv.y = static_cast<float>(uv.y + uv.h) / m_icon_atlas_height;
	vtc.vtx.push_back(vtx);

	size_t i = vtc.vtx.size() - 1;
	vtc.idx.push_back(i - 3);
	vtc.idx.push_back(i - 2);
	vtc.idx.push_back(i - 1);
	vtc.idx.push_back(i - 2);
	vtc.idx.push_back(i - 0);
	vtc.idx.push_back(i - 1);
	end();
}

void gl::ctx::puts(const glm::vec2 &pos, const glm::vec4 color, const char *ch)
{
	begin();

	glm::vec2 dpos = pos;

	for(const char *s = ch; *s; s++) {

		tahoma12::position uv = tahoma12::pc[*s];

		gl::vtx vtx;
		vtx.color = color;

		dpos.x += uv.left;
		dpos.y -= uv.top;

		// top left
		vtx.pos = dpos;
		vtx.uv.x = static_cast<float>(uv.x) / m_font_atlas_width;
		vtx.uv.y = static_cast<float>(uv.y) / m_font_atlas_height;
		m_solid_vtx.push_back(vtx);

		// top right
		vtx.pos.x = dpos.x + uv.w;
		vtx.pos.y = dpos.y;
		vtx.uv.x = static_cast<float>(uv.x + uv.w) / m_font_atlas_width;
		vtx.uv.y = static_cast<float>(uv.y) / m_font_atlas_height;
		m_solid_vtx.push_back(vtx);

		// bottom left
		vtx.pos.x = dpos.x;
		vtx.pos.y = dpos.y + uv.h;
		vtx.uv.x = static_cast<float>(uv.x) / m_font_atlas_width;
		vtx.uv.y = static_cast<float>(uv.y + uv.h) / m_font_atlas_height;
		m_solid_vtx.push_back(vtx);

		// bottom right
		vtx.pos.x = dpos.x + uv.w;
		vtx.pos.y = dpos.y + uv.h;
		vtx.uv.x = static_cast<float>(uv.x + uv.w) / m_font_atlas_width;
		vtx.uv.y = static_cast<float>(uv.y + uv.h) / m_font_atlas_height;
		m_solid_vtx.push_back(vtx);

		size_t i = m_solid_vtx.size() - 1;
		m_solid_idx.push_back(i - 3);
		m_solid_idx.push_back(i - 2);
		m_solid_idx.push_back(i - 1);
		m_solid_idx.push_back(i - 2);
		m_solid_idx.push_back(i - 0);
		m_solid_idx.push_back(i - 1);

		dpos.x -= uv.left;
		dpos.y += uv.top;
		dpos.x += uv.xadvance;
	}

	end();
}


gl::ctx::ctx()
{
	int version = gladLoaderLoadGL();
	assert(version != 0);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_MULTISAMPLE);

	// setup grid objects
	static const char *grid_fs_src = R"(
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

	static const char *grid_vs_src = R"(
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

	m_grid_program = compileshaders(grid_fs_src, grid_vs_src);
	/* fullscreen rect. don't bother with EBO. */
	static float grid_vertices[] = {
		 1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f
	};

	glUseProgram(m_grid_program);
	GLint spacing = glGetUniformLocation(m_grid_program, "spacing");
	glUniform1f(spacing, static_cast<float>(GRID_SPACING));

	glGenBuffers(1, &m_grid_vtxbuf);
	glGenVertexArrays(1, &m_grid_vao);

	glBindVertexArray(m_grid_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_grid_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), grid_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// setup solid geometry objects
	static const char *solid_vs_src = R"(
		#version 330 core

		layout (location = 0) in vec2 pos;
		layout (location = 1) in vec4 color;
		layout (location = 2) in vec2 uv;

		out vec4 a_color;
		out vec2 a_uv;

		uniform mat4 mvp;

		void main()
		{
			gl_Position = mvp * vec4(pos, 0.0, 1.0);
			a_color = color;
			a_uv = uv;
		}
	)";

	static const char *solid_fs_src = R"(
		#version 330 core

		out vec4 FragColor;
		in vec4 a_color;
		in vec2 a_uv;

		uniform sampler2D u_texture;

		void main()
		{
			FragColor = a_color;

			if(a_uv.x != 0.0 && a_uv.y != 0.0) {
				FragColor.a = a_color.a * texture(u_texture, a_uv).r;
			}
		}
	)";

	m_solid_program = compileshaders(solid_fs_src, solid_vs_src);

	glGenBuffers(1, &m_vtxbuf);
	glGenBuffers(1, &m_idxbuf);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(gl::vtx), (void *)offsetof(gl::vtx, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(gl::vtx), (void *)offsetof(gl::vtx, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gl::vtx), (void *)offsetof(gl::vtx, uv));
	glEnableVertexAttribArray(2);

	// setup texture geometry objects
	static const char *texture_vs_src = R"(
		#version 330 core

		layout (location = 0) in vec2 pos;
		layout (location = 1) in vec4 color;
		layout (location = 2) in vec2 uv;

		out vec4 a_color;
		out vec2 a_uv;

		uniform mat4 mvp;

		void main()
		{
			gl_Position = mvp * vec4(pos, 0.0, 1.0);
			a_color = color;
			a_uv = uv;
		}
	)";

	static const char *texture_fs_src = R"(
		#version 330 core

		out vec4 FragColor;

		in vec4 a_color;
		in vec2 a_uv;

		uniform sampler2D u_texture;

		void main()
		{
			FragColor = a_color * texture(u_texture, a_uv);
		}
	)";

	m_texture_program = compileshaders(texture_fs_src, texture_vs_src);

	// icon atlas
	glGenTextures(1, &m_icon_atlas);
	glBindTexture(GL_TEXTURE_2D, m_icon_atlas);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int w, h, nchan;
	unsigned char *data;
	data = stbi_load_from_memory(ia::data, sizeof(ia::data), &w, &h, &nchan, 4);
	assert(data);
	assert(nchan == 4);

	m_icon_atlas_width = w;
	m_icon_atlas_height = h;

	unsigned char *icon = new unsigned char[w * h * nchan];
	for(size_t x = 0; x < ia::icon.w; x++) {
		for(size_t y = 0; y < ia::icon.h; y++) {
			size_t dst = (y * ia::icon.w + x) * nchan;
			size_t src = ((y + ia::icon.y) * w + (x + ia::icon.x)) * nchan;
			for(size_t p = 0; p < nchan; p++) {
				icon[dst + p] = data[src + p];
			}
		}
	}

	set_window_icon(icon, ia::icon.w, ia::icon.h);

	delete[] icon;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	// font atlas
	glGenTextures(1, &m_font_atlas);
	glBindTexture(GL_TEXTURE_2D, m_font_atlas);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	data = stbi_load_from_memory(tahoma12::data, sizeof(tahoma12::data), &w, &h, &nchan, 1);
	assert(data);
	assert(nchan == 1);

	m_font_atlas_width = w;
	m_font_atlas_height = h;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}
