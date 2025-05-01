#ifndef _MAINFRAME_HPP
#define _MAINFRAME_HPP


#include <wx/wx.h>
#include <wx/notebook.h>
#include "src/util/singleton.hpp"


class MainFrame : public wxFrame,
                  public Singleton<MainFrame>
{
public:
	MainFrame();
private:
	void OnNew(wxCommandEvent &e);
	void OnOpen(wxCommandEvent &e);
	void OnSave(wxCommandEvent &e);
	void OnSaveAs(wxCommandEvent &e);
	void OnExit(wxCommandEvent &e);
	void OnUndo(wxCommandEvent &e);
	void OnRedo(wxCommandEvent &e);
private:
	wxNotebook *m_sidebook;
public:
	void Sidebook_Hide()
	{
		m_sidebook->Hide();
		GetSizer()->Layout();
	}
	void Sidebook_Show()
	{
		m_sidebook->Show();
		GetSizer()->Layout();
	}
};

#endif
