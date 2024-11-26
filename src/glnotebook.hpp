#ifndef _GLNOTEBOOK_HPP
#define _GLNOTEBOOK_HPP

#include <wx/wx.h>
#include <wx/notebook.h>

#include <wx/aui/aui.h>

class wxGLContext;

class GLNoteBook : public wxAuiNotebook
{
public:
	GLNoteBook(wxWindow *parent, wxWindowID id = wxID_ANY);
	virtual ~GLNoteBook();
	bool AddCanvas(const wxString &name);
	bool SetCurrent(wxGLCanvas *canvas) { return m_context->SetCurrent(*canvas); }
private:
	wxGLContext *m_context = nullptr;
};

#endif