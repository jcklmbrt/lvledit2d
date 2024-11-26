#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "src/toolbar.hpp"
#include "src/glnotebook.hpp"

class MainFrame : public wxFrame
{
public:
	MainFrame();
private:
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	GLNoteBook *m_notebook = nullptr;
};

#endif
