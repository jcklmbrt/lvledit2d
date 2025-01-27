#ifndef _NOTEBOOK_HPP
#define _NOTEBOOK_HPP

#include <wx/aui/aui.h>
#include <wx/filename.h>

class HistoryList;
class wxGLCanvas;
class GLContext;
class GLCanvas;

class Notebook : public wxAuiNotebook
{
public:
	Notebook(wxWindow *parent, HistoryList *hlist);
	virtual ~Notebook();
	void Save(bool force_rename);
	bool AddCanvas();
	bool AddCanvas(const wxFileName &file);
	bool AddCanvas(GLCanvas *canvas);
private:
	void OnPageChange(wxAuiNotebookEvent &e);
	void OnPageClose(wxAuiNotebookEvent &e);
private:
	HistoryList *m_hlist;
	GLContext *m_context = nullptr;
};

#endif
