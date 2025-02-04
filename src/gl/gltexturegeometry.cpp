#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/bitmap.h>
#include "glad/gl.h"
#include "src/gl/texture.hpp"
#include "src/gl/glcanvas.hpp"
#include "src/gl/glcontext.hpp"
#include "src/edit/editorcontext.hpp"
#include "src/texturepanel.hpp"
#include "src/gl/gltexturegeometry.hpp"

void GLTextureGeometry::Init()
{
	static const char *vs_src = R"(
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

	static const char *fs_src = R"(
		#version 330 core

		out vec4 FragColor;

		in vec4 a_color;
		in vec2 a_uv;

		uniform sampler2D u_texture;

		void main()
		{
			FragColor = texture(u_texture, a_uv);
		}
	)";

	m_program = GLContext::CompileShaders(fs_src, vs_src);

	glGenBuffers(1, &m_vtxbuf);
	glGenBuffers(1, &m_idxbuf);
	glGenVertexArrays(1, &m_vao);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void *)offsetof(TextureVertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void *)offsetof(TextureVertex, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void *)offsetof(TextureVertex, uv));
	glEnableVertexAttribArray(2);
}


GLTextureGeometry::~GLTextureGeometry()
{
	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteBuffers(1, &m_idxbuf);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}


void GLTextureGeometry::CopyBuffersAndDrawElements()
{
	glUseProgram(m_program);
	glBindVertexArray(m_vao);

	for(auto &[tex_id, vtc] : m_batches) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
		glBufferData(GL_ARRAY_BUFFER, vtc.vtx.size() * sizeof(TextureVertex), vtc.vtx.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vtc.idx.size() * sizeof(GLuint), vtc.idx.data(), GL_STATIC_DRAW);

		glBindTexture(GL_TEXTURE_2D, tex_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
		glDrawElements(GL_TRIANGLES, vtc.idx.size(), GL_UNSIGNED_INT, 0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


void GLTextureGeometry::ClearBuffers()
{
	m_batches.clear();
}


void GLTextureGeometry::SetMatrices(const Matrix4 &proj, const Matrix4 &view)
{
	glUseProgram(m_program);
	Matrix4 mvp = proj * view;
	int pos = glGetUniformLocation(m_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));
}


static TextureVertex SetVertex(const Point2D &pt, const Color &color, const Rect2D &aabb)
{
	Point2D Mins = aabb.mins;
	Point2D Size = aabb.maxs - aabb.mins;

	TextureVertex Vertex;
	Vertex.position = pt;
	Vertex.color = color;
	Vertex.uv = (pt - Mins) / Size;
	return Vertex;
}


void GLTextureGeometry::AddPolygon(const Point2D pts[], size_t npts, const Rect2D &uv, GLTexture &texture, const Color &color)
{
	if(!glIsTexture(texture.gltex)) {
		texture.InitTextureObject();
	}

	TextureVertices &vtc = m_batches[texture.gltex];

	TextureVertex start = SetVertex(pts[0], color, uv);

	vtc.vtx.push_back(start);
	size_t start_idx = vtc.vtx.size() - 1;

	for(size_t i = 1; i < npts; i++) {
		Point2D a = pts[i];
		Point2D b = pts[(i + 1) % npts];
		vtc.vtx.push_back(SetVertex(a, color, uv));
		vtc.vtx.push_back(SetVertex(b, color, uv));

		vtc.idx.push_back(start_idx);
		vtc.idx.push_back(start_idx + (i * 2) - 1);
		vtc.idx.push_back(start_idx + (i * 2));
	}
}


void GLTextureGeometry::AddPolygon(const ConvexPolygon &poly, const Color &color)
{
	GLTexture *texture = poly.GetTexture();
	if(texture == nullptr) {
		return;
	}

	const std::vector<Point2D> &pts = poly.points;

	const Rect2D &aabb = poly.GetUV();

	size_t npoints = pts.size();
	TextureVertex start = SetVertex(pts[0], color, aabb);

	if(!glIsTexture(texture->gltex)) {
		texture->InitTextureObject();
	}

	TextureVertices &vtc = m_batches[texture->gltex];

	vtc.vtx.push_back(start);
	size_t start_idx = vtc.vtx.size() - 1;

	for(size_t i = 1; i < npoints; i++) {
		Point2D a = pts[i];
		Point2D b = pts[(i + 1) % npoints];
		vtc.vtx.push_back(SetVertex(a, color, aabb));
		vtc.vtx.push_back(SetVertex(b, color, aabb));

		vtc.idx.push_back(start_idx);
		vtc.idx.push_back(start_idx + (i * 2) - 1);
		vtc.idx.push_back(start_idx + (i * 2));
	}
}
