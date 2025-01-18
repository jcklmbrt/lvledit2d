#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP

#include <wx/event.h>
#include <wx/wx.h>
#include <wx/aui/aui.h>

class HistoryList;

class MainFrame : public wxFrame
{
	enum ID : int {
		/* private */
		MASK = 0x100,
		/* public */
		SHOW_HISTORY 
	};
public:
	MainFrame();
	inline wxAuiNotebook *GetNotebook() 
	{ 
		return m_notebook; 
	}
	inline HistoryList *GetHistroyList()
	{
		return m_historylist;
	}
private:
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	void OnUndo(wxCommandEvent &e);
	void OnRedo(wxCommandEvent &e);
	void OnShowHistory(wxCommandEvent &e);
	wxAuiNotebook *m_notebook = nullptr;
	HistoryList *m_historylist = nullptr;
	wxPanel *m_sidepanel = nullptr;
};

#endif
