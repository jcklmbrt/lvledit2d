#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP

#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "src/toolbar.hpp"

class MainFrame : public wxFrame
{
public:
	MainFrame();
private:
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	wxAuiNotebook *m_notebook = nullptr;
};

#endif
