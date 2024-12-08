#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP

#include <wx/aui/auibook.h>
#include <wx/wx.h>
#include <wx/aui/aui.h>

class MainFrame : public wxFrame
{
public:
	MainFrame();
	inline wxAuiNotebook *GetNotebook() 
	{ 
		return m_notebook; 
	}
private:
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	wxAuiNotebook *m_notebook = nullptr;
};

#endif
