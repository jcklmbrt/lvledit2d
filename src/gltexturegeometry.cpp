#include <wx/image.h>
#include <wx/listctrl.h>
#include <wx/bitmap.h>
#include "src/glcontext.hpp"
#include "src/texturepanel.hpp"
#include "src/gltexturegeometry.hpp"

GLTextureGeometry::GLTextureGeometry()
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

	CopyBuffers();
}


GLTextureGeometry::~GLTextureGeometry()
{
	glDeleteBuffers(1, &m_vtxbuf);
	glDeleteBuffers(1, &m_idxbuf);
	glDeleteVertexArrays(1, &m_vao);
	glDeleteProgram(m_program);
}


void GLTextureGeometry::CopyBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vtxbuf);
	glBufferData(GL_ARRAY_BUFFER, m_vtx.size() * sizeof(TextureVertex), m_vtx.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_idx.size() * sizeof(GLuint), m_idx.data(), GL_STATIC_DRAW);
}


void GLTextureGeometry::DrawElements(Texture &texture)
{
	texture.Bind();

	glUseProgram(m_program);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_idxbuf);
	glDrawElements(GL_TRIANGLES, m_idx.size(), GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}


void GLTextureGeometry::ClearBuffers()
{
	m_idx.clear();
	m_vtx.clear();
}


void GLTextureGeometry::SetMatrices(const Matrix4 &proj, const Matrix4 &view)
{
	glUseProgram(m_program);
	Matrix4 mvp = proj * view;
	int pos = glGetUniformLocation(m_program, "mvp");
	glUniformMatrix4fv(pos, 1, GL_FALSE, glm::value_ptr(mvp));
}


static TextureVertex SetVertex(const Point2D &pt, const Color &color, float spacing)
{
	TextureVertex vtx;
	vtx.position = pt;
	vtx.color = color;
	vtx.uv = pt / spacing;
	return vtx;
}


static TextureVertex SetVertex(const Point2D &pt, const Color &color, const Rect2D &aabb)
{
	Point2D lt = aabb.GetLeftTop();
	Point2D size = aabb.GetSize();

	TextureVertex vtx;
	vtx.position = pt;
	vtx.color = color;
	vtx.uv = (pt - lt) / size;
	return vtx;
}


void GLTextureGeometry::AddPolygon(const Point2D pts[], size_t npts, Texture &texture, const Color &color)
{
	Rect2D aabb;
	aabb.FitPoints(pts, npts);

	TextureVertex start = SetVertex(pts[0], color, aabb);

	m_vtx.push_back(start);
	size_t start_idx = m_vtx.size() - 1;

	for(size_t i = 1; i < npts; i++) {
		Point2D a = pts[i];
		Point2D b = pts[(i + 1) % npts];
		m_vtx.push_back(SetVertex(a, color, aabb));
		m_vtx.push_back(SetVertex(b, color, aabb));

		m_idx.push_back(start_idx);
		m_idx.push_back(start_idx + (i * 2) - 1);
		m_idx.push_back(start_idx + (i * 2));
	}

	CopyBuffers();
	DrawElements(texture);
	ClearBuffers();
}


void GLTextureGeometry::AddPolygon(const ConvexPolygon &poly, const Color &color)
{
	Texture *texture = poly.GetTexture();
	if(texture == nullptr) {
		return;
	}

	const std::vector<Point2D> &pts = poly.GetPoints();

	const Rect2D &aabb = poly.GetAABB();

	size_t npoints = pts.size();
	TextureVertex start = SetVertex(pts[0], color, aabb);

	m_vtx.push_back(start);
	size_t start_idx = m_vtx.size() - 1;

	for(size_t i = 1; i < npoints; i++) {
		Point2D a = pts[i];
		Point2D b = pts[(i + 1) % npoints];
		m_vtx.push_back(SetVertex(a, color, aabb));
		m_vtx.push_back(SetVertex(b, color, aabb));

		m_idx.push_back(start_idx);
		m_idx.push_back(start_idx + (i * 2) - 1);
		m_idx.push_back(start_idx + (i * 2));
	}

	CopyBuffers();
	DrawElements(*texture);
	ClearBuffers();
}

Texture::Texture(const wxFileName &filename)
	: m_filename(filename)
{
	wxASSERT(filename.IsOk() && filename.Exists());

	wxImage img(filename.GetFullPath());

	wxASSERT(img.IsOk());

	if(img.HasAlpha()) {
		m_pixelwidth = 4;
	} else {
		m_pixelwidth = 3;
	}

	m_width = img.GetWidth();
	m_height = img.GetHeight();
	size_t npix = m_width * m_height;
	m_data = new unsigned char[npix * m_pixelwidth];

	unsigned char *alpha = img.GetAlpha();
	unsigned char *data = img.GetData();

	for(size_t i = 0; i < npix; i++) {
		m_data[i * m_pixelwidth + 0] = data[i * 3 + 0];
		m_data[i * m_pixelwidth + 1] = data[i * 3 + 1];
		m_data[i * m_pixelwidth + 2] = data[i * 3 + 2];
		if(m_pixelwidth == 4) {
			m_data[i * m_pixelwidth + 3] = alpha[i];
		}
	}

	TextureList *tlist = TextureList::GetInstance();
	wxImageList *ilist = tlist->GetImageList(wxIMAGE_LIST_SMALL);

	wxBitmap bmp(img);
	wxBitmap::Rescale(bmp, wxSize(THUMB_SIZE_X, THUMB_SIZE_Y));
	m_index = ilist->Add(bmp);
}


void Texture::InitTextureObject()
{
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(m_pixelwidth == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, m_data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, m_data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture()
{
	//glDeleteTextures(1, &m_texture);
	//wxASSERT_MSG(true, "We shouldn't be deleting textures (yet)");
	//wxMessageBox("Destructor Called");
	//delete[] m_data;
}

void Texture::Bind()
{
	if(!glIsTexture(m_texture)) {
		InitTextureObject();
	}

	glBindTexture(GL_TEXTURE_2D, m_texture);
}
