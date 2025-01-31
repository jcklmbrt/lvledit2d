#include <cstddef>
#include <glad/gl.h>
#include <wx/wx.h>

#include "src/glsolidgeometry.hpp"
#include "src/glbackgroundgrid.hpp"
#include "src/gltexturegeometry.hpp"
#include "src/geometry.hpp"
#include "src/glcontext.hpp"


GLuint GLContext::CompileShaders(const char *fs_src, const char *vs_src)
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
	m_solid->SetMatrices(proj, view);
	m_texture->SetMatrices(proj, view);
	m_grid->SetMatrices(proj, view);
}

void GLContext::DrawElements()
{
	m_grid->DrawGrid();
	//m_texture->DrawElements();
	m_solid->DrawElements();
}


void GLContext::ClearBuffers()
{
	m_solid->ClearBuffers();
	//m_texture->ClearBuffers();
}


void GLContext::AddRect(const Rect2D &rect, const Color &color)
{
	m_solid->AddRect(rect, color);
}


void GLContext::AddLine(const Point2D &a, const Point2D &b, float thickness, const Color &color)
{
	m_solid->AddLine(a, b, thickness, color);
}

void GLContext::AddPolygon(const ConvexPolygon &poly, const Color &color)
{
	m_texture->AddPolygon(poly, color);
}

void GLContext::AddPolygon(const Point2D pts[], size_t npts, Texture &texture, const Color &color)
{
	m_texture->AddPolygon(pts, npts, texture, color);
}


void GLContext::CopyBuffers()
{
	m_solid->CopyBuffers();
	m_texture->CopyBuffers();
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

	m_grid = new GLBackgroundGrid();
	m_solid = new GLSolidGeometry();
	m_texture = new GLTextureGeometry();
}


GLContext::~GLContext()
{
	delete m_grid;
	delete m_solid;
	delete m_texture;
}

