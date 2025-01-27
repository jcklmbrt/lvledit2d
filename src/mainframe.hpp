#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP


#include <wx/wx.h>


class HistoryList;
class Notebook;


class MainFrame : public wxFrame
{
public:
	enum ID : int {
		/* private */
		MASK = 0x100,
		/* public */
		SHOW_HISTORY,
	};
public:
	MainFrame();
	inline Notebook *GetNotebook()
	{ 
		return m_notebook; 
	}
	inline HistoryList *GetHistoryList()
	{
		return m_historylist;
	}
private: /* Events */
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnSaveAs(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	void OnUndo(wxCommandEvent &e);
	void OnRedo(wxCommandEvent &e);
	void OnShowHistory(wxCommandEvent &e);
	Notebook *m_notebook = nullptr;
	HistoryList *m_historylist = nullptr;
};

#endif
