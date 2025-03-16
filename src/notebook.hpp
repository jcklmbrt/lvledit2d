#ifndef _NOTEBOOK_HPP
#define _NOTEBOOK_HPP

#include <wx/aui/aui.h>
#include <wx/filename.h>
#include "src/singleton.hpp"

class HistoryList;
class wxGLCanvas;
class GLContext;
class GLCanvas;

class Notebook : public wxAuiNotebook,
                 public Singleton<Notebook>
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
};

#endif
