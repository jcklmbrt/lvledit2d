
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>

#include "src/mainframe.hpp"
#include "src/glcanvas.hpp"
#include "src/glnotebook.hpp"

#include "res/icon.xpm"
#include "res/line.xpm"
#include "res/quad.xpm"
#include "res/texture.xpm"
#include "res/reset.xpm"
#include "res/pawn.xpm"
#include "res/hand.xpm"
#include "res/save.xpm"



MainFrame::MainFrame()
	: wxFrame(nullptr, wxID_ANY, "lvledit2d")
{
	SetIcon(icon_xpm);
	Maximize(true);

	wxMenu *file = new wxMenu;
	file->Append(wxID_EXIT);

	wxMenuBar *menubar = new wxMenuBar;
	menubar->Append(file, "&File");
	SetMenuBar(menubar);

	wxStatusBar *status = CreateStatusBar();

	wxToolBar *toolbar = CreateToolBar(wxTB_LEFT | wxTB_VERTICAL);

	toolbar->AddTool(wxID_ANY, "Select", hand_xpm);
	toolbar->AddTool(wxID_ANY, "Line", line_xpm);
	toolbar->AddTool(wxID_ANY, "Quad", quad_xpm);
	toolbar->AddTool(wxID_ANY, "Texture", texture_xpm);
	toolbar->AddTool(wxID_ANY, "Entity", pawn_xpm);
	toolbar->AddTool(wxID_ANY, "Save", save_xpm);
	toolbar->AddTool(wxID_ANY, "Reset", reset_xpm);
	toolbar->Realize();

	wxPanel    *panel    = new wxPanel(this);
	GLNoteBook *notebook = new GLNoteBook(panel, wxID_ANY);

	wxBoxSizer *sizer;

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(notebook, 1, wxEXPAND);
	panel->SetSizer(sizer);

	for(int i = 0; i < 10; i++) {
		wxString name = wxString::Format("Page %d", i + 1);
		notebook->AddCanvas(name);
	}

	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(panel, 1, wxEXPAND);
	SetSizer(sizer);

	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
}

void MainFrame::OnExit(wxCommandEvent &e)
{
	Close(true);
}
