
#include "src/edit/editorcontext.hpp"
#include "src/toolbar.hpp"
#include "src/mainframe.hpp"
#include "src/historylist.hpp"
#include "src/lvledit2d.hpp"
#include "src/glcanvas.hpp"
#include "src/notebook.hpp"

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
	file->Append(wxID_SAVEAS);
	file->Append(save);
	file->Append(exit);

	wxMenuItem *undo = new wxMenuItem(edit, wxID_UNDO);
	wxMenuItem *redo = new wxMenuItem(edit, wxID_REDO);
	wxMenuItem *show_history = new wxMenuItem(edit, ID::SHOW_HISTORY, "Show History", "", wxITEM_CHECK);
	edit->Append(show_history);
	edit->Append(undo);
	edit->Append(redo);

	wxMenuBar *menubar = new wxMenuBar;
	menubar->Append(file, "&File");
	menubar->Append(edit, "&Edit");
	SetMenuBar(menubar);

	wxToolBar *toolbar = new ToolBar(this, wxID_ANY);
	SetToolBar(toolbar);

	HistoryList *hlist = new HistoryList(this);
	Notebook *notebook = new Notebook(this, hlist);

	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(notebook, 1, wxEXPAND);
	sizer->Add(hlist, 0, wxEXPAND);
	SetSizer(sizer);

	m_notebook = notebook;
	m_historylist = hlist;

	m_historylist->Hide();

	Bind(wxEVT_MENU, &MainFrame::OnShowHistory, this, ID::SHOW_HISTORY);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
	Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, wxID_SAVEAS);
	Bind(wxEVT_MENU, &MainFrame::OnNew,  this, wxID_NEW);
	Bind(wxEVT_MENU, &MainFrame::OnUndo, this, wxID_UNDO);
	Bind(wxEVT_MENU, &MainFrame::OnRedo, this, wxID_REDO);
}


void MainFrame::OnOpen(wxCommandEvent &e)
{
	wxFileDialog dialog(this, "Open L2D file", wxEmptyString, wxEmptyString,
		"L2D files (*.l2d)|*.l2d", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if(dialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	m_notebook->AddCanvas(dialog.GetPath());
}


void MainFrame::OnSave(wxCommandEvent &e)
{
	m_notebook->Save(false);
}


void MainFrame::OnSaveAs(wxCommandEvent &e)
{
	m_notebook->Save(true);
}


void MainFrame::OnNew(wxCommandEvent &e)
{
	m_notebook->AddCanvas();
}


void MainFrame::OnShowHistory(wxCommandEvent &e)
{
	wxSizer *sizer = GetSizer();
	if(e.IsChecked()) {
		m_historylist->Show();
	} else {
		m_historylist->Hide();
	}

	sizer->Layout();

	Refresh();
}


void MainFrame::OnUndo(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		EditorContext &editor = canvas->GetEditor();
		editor.Undo();
		canvas->Refresh();
	}
}


void MainFrame::OnRedo(wxCommandEvent &e)
{
	GLCanvas *canvas = GLCanvas::GetCurrent();
	if(canvas != nullptr) {
		EditorContext &editor = canvas->GetEditor();
		editor.Redo();
		canvas->Refresh();
	}
}


void MainFrame::OnExit(wxCommandEvent &e)
{
	Close(true);
}
