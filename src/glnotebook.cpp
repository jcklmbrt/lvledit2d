
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "src/glcanvas.hpp"
#include "src/glnotebook.hpp"


GLNoteBook::GLNoteBook(wxWindow *parent, wxWindowID id)
	: wxAuiNotebook(parent, id)
{
}

GLNoteBook::~GLNoteBook()
{
	if(m_context != nullptr) {
		delete m_context;
	}
}

bool GLNoteBook::AddCanvas(const wxString &name)
{
	wxGLAttributes attrs;
	attrs.PlatformDefaults().Defaults().EndList();
	bool accepted = wxGLCanvas::IsDisplaySupported(attrs);

	GLCanvas *canvas = new GLCanvas(this, attrs);

	if(m_context == nullptr) {
		wxGLContextAttrs ctxattrs;
		ctxattrs.PlatformDefaults().CoreProfile().OGLVersion(2, 0).EndList();
		m_context = new wxGLContext(canvas, nullptr, &ctxattrs);
	}

	if(!m_context->IsOK()) {
		wxMessageBox("OpenGL Version Error");
		delete m_context;
		m_context = nullptr;
	}

	return AddPage(canvas, name, true);
}