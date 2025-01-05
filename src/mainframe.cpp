
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include <wx/wfstream.h>


#include "src/drawpanel.hpp"
#include "src/toolbar.hpp"
#include "src/mainframe.hpp"


#include "res/reset.xpm"
#include "res/save.xpm"
#include "res/icon.xpm"


MainFrame::MainFrame()
	: wxFrame(nullptr, wxID_ANY, "LvlEdit2d")
{
	SetIcon(icon_xpm);
	Maximize(true);

	wxMenu *file = new wxMenu;
	wxMenu *edit = new wxMenu;

	wxMenuItem *save = new wxMenuItem(file, wxID_SAVE);
	save->SetBitmap(save_xpm);
	wxMenuItem *exit = new wxMenuItem(file, wxID_EXIT);
	exit->SetBitmap(reset_xpm);

	file->Append(wxID_NEW);
	file->Append(wxID_OPEN);
	file->Append(save);
	file->Append(exit);

	wxMenuBar *menubar = new wxMenuBar;
	menubar->Append(file, "&File");
	menubar->Append(edit, "&Edit");
	SetMenuBar(menubar);

	wxStatusBar *status  = CreateStatusBar();

	wxToolBar *toolbar = new ToolBar(this, wxID_ANY);
	SetToolBar(toolbar);

	wxPanel       *panel    = new wxPanel(this);
	wxAuiNotebook *notebook = new wxAuiNotebook(panel, wxID_ANY);

	wxBoxSizer *sizer;

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(notebook, 1, wxEXPAND);
	panel->SetSizer(sizer);

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(panel, 1, wxEXPAND);
	SetSizer(sizer);

	m_notebook = notebook;

	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
	Bind(wxEVT_MENU, &MainFrame::OnNew,  this, wxID_NEW);
}


void MainFrame::OnOpen(wxCommandEvent &e)
{
	wxFileDialog dialog(this);

	if(dialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	DrawPanel *dp = new DrawPanel(m_notebook);
	m_notebook->AddPage(dp, dialog.GetFilename(), true);
}


void MainFrame::OnSave(wxCommandEvent &e)
{
	wxFileDialog dialog(this, "Save file", "", "",
		"LvlEdit2d files (*.l2d)|*.l2d", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if(dialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	int sel = m_notebook->GetSelection();
	m_notebook->DeletePage(sel);
}


void MainFrame::OnNew(wxCommandEvent &e)
{
	static int n = 0;
	wxString s;
	s.Printf("Page:%d", n++);
	DrawPanel *dp = new DrawPanel(m_notebook);
	m_notebook->AddPage(dp, s, true);
}


void MainFrame::OnExit(wxCommandEvent &e)
{
	Close(true);
}
